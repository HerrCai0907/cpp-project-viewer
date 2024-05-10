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

class Task::Impl {
  friend Scheduler;

  std::size_t m_id{};
  std::uint8_t m_priority;
  std::function<void()> m_fn;
  std::size_t m_pre_count;
  std::size_t m_finished_pre_cont{0u};
  bool m_is_finished{false};
  std::vector<Impl *> m_post_tasks{};
  std::mutex m_mutex{}; // lock order should be from pre to post
  std::condition_variable m_cv{};

public:
  Impl(std::uint8_t priority, std::function<void()> fn,
       std::vector<Task *> const &pre_tasks, Scheduler &scheduler)
      : m_id{get_task_id()}, m_priority(priority), m_fn{std::move(fn)},
        m_pre_count{pre_tasks.size()} {
    std::size_t finished_pre = 0;
    for (Task *pre : pre_tasks) {
      if (!pre->m_impl->insert_post()) {
        finished_pre++;
      }
    }
    update_finished_pre_count(finished_pre, scheduler);
  }
  void wait() {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_cv.wait(lock, [this] { return m_is_finished; });
  }

private:
  void run(Scheduler &scheduler) {
    m_fn();
    post_run(scheduler);
    m_cv.notify_all();
  }
  void post_run(Scheduler &scheduler) {
    std::lock_guard<std::mutex> lock{m_mutex};
    for (Impl *post : m_post_tasks) {
      post->update_finished_pre_count(1, scheduler);
    }
    m_is_finished = true;
  }
  bool insert_post() {
    std::lock_guard<std::mutex> lock{m_mutex};
    if (!m_is_finished) {
      m_post_tasks.push_back(this);
      return true;
    }
    return false;
  }
  void update_finished_pre_count(std::size_t finished_count,
                                 Scheduler &scheduler) {
    std::lock_guard<std::mutex> lock{m_mutex};
    m_finished_pre_cont += finished_count;
    if (m_finished_pre_cont == m_pre_count) {
      m_is_finished = false;
      scheduler.mark_task_ready(this);
    }
  }
};

Task::Task(std::uint8_t priority, std::function<void()> fn,
           std::vector<Task *> const &pre_tasks, Scheduler &scheduler)
    : m_impl(new Impl(priority, std::move(fn), pre_tasks, scheduler)) {}

Task::Task(Task &&other) = default;

Task &Task::operator=(Task &&other) = default;

Task::~Task() = default;

void Task::wait() { m_impl->wait(); }

class Scheduler::ReadyQueue {
  struct CompareTask {
    bool operator()(Task::Impl *a, Task::Impl *b) {
      return a->m_priority < b->m_priority;
    }
  };

  std::priority_queue<Task::Impl *, std::vector<cpjview::Task::Impl *>,
                      CompareTask>
      m_queue{};
  std::mutex m_mutex{};
  std::condition_variable m_cv{};

public:
  void push(Task::Impl *task) {
    {
      std::lock_guard<std::mutex> lock{m_mutex};
      m_queue.push(task);
    }
    m_cv.notify_one();
  }
  size_t size() const { return m_queue.size(); }
  bool empty() const { return m_queue.empty(); }

  Task::Impl *pop(std::atomic_bool const &force_stop_flag) {
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
    Task::Impl *top = m_queue.top();
    m_queue.pop();
    return top;
  }

  void notify() { m_cv.notify_all(); }
};

Scheduler::ThreadWrapper::ThreadWrapper(Scheduler &scheduler)
    : m_thread{}, m_stop_flag{false} {
  m_thread = std::thread([this, &scheduler]() {
    while (!m_stop_flag) {
      Task::Impl *task = scheduler.pop_ready_task(m_stop_flag);
      if (task != nullptr) {
        task->run(scheduler);
      }
    }
  });
}

Scheduler::Scheduler(std::size_t executor_count)
    : m_ready_queue(new ReadyQueue()) {
  for (std::size_t index = 0; index < executor_count; index++) {
    m_thread_pool.push_back(new ThreadWrapper(*this));
  }
}

Scheduler::~Scheduler() {
  for (ThreadWrapper *wrapper : m_thread_pool) {
    wrapper->m_stop_flag = true;
  }
  m_ready_queue->notify();
  for (ThreadWrapper *wrapper : m_thread_pool) {
    if (wrapper->m_thread.joinable()) {
      wrapper->m_thread.join();
    }
  }
  for (ThreadWrapper *wrapper : m_thread_pool) {
    delete wrapper;
  }
}

void Scheduler::mark_task_ready(Task::Impl *task) {
  m_ready_queue->push(task);
  spdlog::trace("[scheduler] add ready task {}, current is {}", task->m_id,
                m_ready_queue->size());
}

Task::Impl *Scheduler::pop_ready_task(std::atomic_bool const &force_stop_flag) {
  Task::Impl *task = m_ready_queue->pop(force_stop_flag);
  spdlog::trace("[scheduler] consume task {}, current is {}", task->m_id,
                m_ready_queue->size());
  return task;
}

} // namespace cpjview
