#include "cpjview/common/scheduler.hpp"
#include "spdlog/spdlog.h"
#include <atomic>
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
           std::vector<Task *> const &pre_tasks, Scheduler &scheduler)
    : m_id{get_task_id()}, m_priority(priority), m_fn{std::move(fn)},
      m_pre_count{pre_tasks.size()} {
  spdlog::trace("[scheduler] create task {} with pre-task size {}", m_id,
                m_pre_count);
  std::size_t finished_pre = 0;
  for (Task *pre : pre_tasks) {
    if (!pre->insert_post(this)) {
      finished_pre++;
    }
  }
  update_finished_pre_count(finished_pre, scheduler);
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
  for (Task *post : m_post_tasks) {
    post->update_finished_pre_count(1, scheduler);
  }
  m_is_finished = true;
}
bool Task::insert_post(Task *post_task) {
  std::lock_guard<std::mutex> lock{m_mutex};
  if (!m_is_finished) {
    m_post_tasks.push_back(post_task);
    return true;
  }
  return false;
}
void Task::update_finished_pre_count(std::size_t finished_count,
                                     Scheduler &scheduler) {
  std::lock_guard<std::mutex> lock{m_mutex};
  m_finished_pre_cont += finished_count;
  if (m_finished_pre_cont == m_pre_count) {
    scheduler.mark_task_ready(this);
  }
}

class Scheduler::ReadyQueue {
  struct CompareTask {
    bool operator()(Task *a, Task *b) { return a->m_priority < b->m_priority; }
  };

  std::priority_queue<Task *, std::vector<cpjview::Task *>, CompareTask>
      m_queue{};
  std::mutex m_mutex{};
  std::condition_variable m_cv{};

public:
  void push(Task *task) {
    {
      std::lock_guard<std::mutex> lock{m_mutex};
      m_queue.push(task);
    }
    m_cv.notify_one();
  }
  size_t size() const { return m_queue.size(); }
  bool empty() const { return m_queue.empty(); }

  Task *pop(std::atomic_bool const &force_stop_flag) {
    enum ResumeReason { None, Stop, NewTask };
    ResumeReason reason = ResumeReason::None;
    std::unique_lock<std::mutex> lock{m_mutex};

    m_cv.wait(lock, [this, &force_stop_flag, &reason]() -> bool {
      if (force_stop_flag) {
        reason = ResumeReason::Stop;
        return true;
      }
      if (!empty()) {
        reason = ResumeReason::NewTask;
        return true;
      }
      return false;
    });
    if (reason == ResumeReason::Stop) {
      return nullptr;
    }
    assert(reason == ResumeReason::NewTask);
    Task *top = m_queue.top();
    m_queue.pop();
    return top;
  }

  void notify() { m_cv.notify_all(); }
};

Scheduler::ThreadWrapper::ThreadWrapper(Scheduler &scheduler)
    : m_thread{}, m_stop_flag{false} {
  m_thread = std::thread([this, &scheduler]() {
    while (!m_stop_flag) {
      Task *task = scheduler.pop_ready_task(m_stop_flag);
      if (task != nullptr) {
        task->run(scheduler);
      }
    }
  });
}

Scheduler::Scheduler(std::size_t executor_count)
    : m_ready_queue(new ReadyQueue()) {
  for (std::size_t index = 0; index < executor_count; index++) {
    m_thread_pool.push_back(std::make_unique<ThreadWrapper>(*this));
  }
}

Scheduler::~Scheduler() {
  for (std::unique_ptr<ThreadWrapper> &wrapper : m_thread_pool) {
    wrapper->m_stop_flag = true;
  }
  m_ready_queue->notify();
  for (std::unique_ptr<ThreadWrapper> &wrapper : m_thread_pool) {
    if (wrapper->m_thread.joinable()) {
      wrapper->m_thread.join();
    }
  }
}

void Scheduler::mark_task_ready(Task *task) {
  m_ready_queue->push(task);
  spdlog::trace("[scheduler] add ready task {}, current size is {}", task->m_id,
                m_ready_queue->size());
}

Task *Scheduler::pop_ready_task(std::atomic_bool const &force_stop_flag) {
  Task *task = m_ready_queue->pop(force_stop_flag);
  if (task != nullptr) {
    spdlog::trace("[scheduler] consume task {}, current size is {}", task->m_id,
                  m_ready_queue->size());
  }
  return task;
}

} // namespace cpjview
