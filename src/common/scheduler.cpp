#include "cpjview/common/scheduler.hpp"
#include "spdlog/spdlog.h"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace cpjview {

static std::size_t get_task_id() {
  static std::size_t id = 0U;
  return id++;
}

Task::Task(std::uint8_t priority, std::function<void()> fn,
           std::size_t pre_count)
    : m_id{get_task_id()}, m_priority(priority), m_fn{std::move(fn)},
      m_pre_count{pre_count} {
  spdlog::trace("[scheduler] create task {} with pre-task size {}", m_id,
                m_pre_count);
}

void Task::sync_with_pre_tasks(
    std::shared_ptr<Task> self,
    std::vector<std::shared_ptr<Task>> const &pre_tasks, Scheduler &scheduler) {
  std::size_t finished_pre = 0;
  for (std::shared_ptr<Task> pre : pre_tasks) {
    if (!pre->insert_post(self)) {
      finished_pre++;
    }
  }
  update_finished_pre_count(self, finished_pre, scheduler);
}

void Task::wait() {
  std::unique_lock<std::mutex> lock{m_mutex};
  m_cv.wait(lock, [this] { return m_is_finished; });
}

void Task::run(Scheduler &scheduler) {
  m_fn();
  post_run(scheduler);
  m_cv.notify_all();
}

void Task::post_run(Scheduler &scheduler) {
  std::lock_guard<std::mutex> lock{m_mutex};
  for (std::shared_ptr<Task> &post : m_post_tasks) {
    update_finished_pre_count(post, 1, scheduler);
  }
  m_is_finished = true;
}

bool Task::insert_post(std::shared_ptr<Task> post_task) {
  std::lock_guard<std::mutex> lock{m_mutex};
  if (!m_is_finished) {
    m_post_tasks.push_back(post_task);
    return true;
  }
  return false;
}

void Task::update_finished_pre_count(std::shared_ptr<Task> &self,
                                     std::size_t finished_count,
                                     Scheduler &scheduler) {
  std::lock_guard<std::mutex> lock{self->m_mutex};
  self->m_finished_pre_cont += finished_count;
  if (self->m_finished_pre_cont == self->m_pre_count) {
    scheduler.mark_task_ready(self);
  }
}

class Scheduler::ReadyQueue {
  struct CompareTask {
    bool operator()(std::shared_ptr<Task> const &a,
                    std::shared_ptr<Task> const &b) {
      return a->m_priority < b->m_priority;
    }
  };

  std::priority_queue<std::shared_ptr<Task>, std::vector<std::shared_ptr<Task>>,
                      CompareTask>
      m_queue{};
  std::mutex m_mutex{};
  std::condition_variable m_cv{};

public:
  void push(std::shared_ptr<Task> task) {
    {
      std::lock_guard<std::mutex> lock{m_mutex};
      m_queue.push(task);
    }
    m_cv.notify_one();
  }
  size_t size() {
    std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.size();
  }
  bool empty() {
    std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.empty();
  }

  std::shared_ptr<Task> pop(std::atomic<Thread::State> const &thread_state) {
    std::unique_lock<std::mutex> lock{m_mutex};

    m_cv.wait(lock, [this, &thread_state]() -> bool {
      return thread_state == Thread::State::Stop || !m_queue.empty();
    });
    switch (thread_state) {
    case Thread::State::Busy: {
      std::shared_ptr<Task> top = m_queue.top();
      m_queue.pop();
      return top;
    }
    case Thread::State::Stop: {
      return nullptr;
    }
    }
  }

  void notify_cv() { m_cv.notify_all(); }
};

Scheduler::Thread::Thread(Scheduler &scheduler)
    : m_thread{}, m_state{State::Busy} {
  m_thread = std::thread([this, &scheduler]() {
    while (m_state != State::Stop) {
      std::shared_ptr<Task> task = scheduler.pop_ready_task(m_state);
      if (task != nullptr) {
        task->run(scheduler);
      }
    }
  });
}

Scheduler::Scheduler(std::size_t executor_count)
    : m_ready_queue(std::make_unique<ReadyQueue>()) {
  for (std::size_t index = 0; index < executor_count; index++) {
    m_thread_pool.push_back(std::make_unique<Thread>(*this));
  }
}

Scheduler::~Scheduler() {
  for (std::unique_ptr<Thread> &th : m_thread_pool) {
    th->m_state = Thread::State::Stop;
  }
  m_ready_queue->notify_cv();
  for (std::unique_ptr<Thread> &wrapper : m_thread_pool) {
    if (wrapper->m_thread.joinable()) {
      wrapper->m_thread.join();
    }
  }
}

void Scheduler::mark_task_ready(std::shared_ptr<Task> task) {
  m_ready_queue->push(task);
  spdlog::trace("[scheduler] add ready task {}, current size is {}", task->m_id,
                m_ready_queue->size());
}

std::shared_ptr<Task>
Scheduler::pop_ready_task(std::atomic<Thread::State> const &thread_state) {
  std::shared_ptr<Task> task = m_ready_queue->pop(thread_state);
  if (task != nullptr) {
    spdlog::trace("[scheduler] consume task {}, current size is {}", task->m_id,
                  m_ready_queue->size());
  }
  return task;
}

} // namespace cpjview
