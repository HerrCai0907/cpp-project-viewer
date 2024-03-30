#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

namespace cpjview {

class Scheduler;

class Task {
  friend Scheduler;
  std::function<void()> m_fn;
  std::size_t m_pre_count;
  std::size_t m_finished_pre_cont{0u};
  bool m_is_finished{false};
  std::vector<Task *> m_post_tasks{};
  std::mutex m_mutex{}; // lock order should be from pre to post
  std::condition_variable m_cv{};

public:
  Task(std::function<void()> fn, std::vector<Task *> const &pre_tasks,
       Scheduler &scheduler);
  void wait();

private:
  void run(Scheduler &scheduler);
  void post_run(Scheduler &scheduler);
  bool insert_post();
  void update_finished_pre_count(std::size_t finished_count,
                                 Scheduler &scheduler);
};

template <class T> class TaskWithRet {
  std::optional<T> m_ret;
  Task m_task;

public:
  TaskWithRet(std::function<T()> fn, std::vector<Task *> const &pre_tasks,
              Scheduler &scheduler)
      : m_ret{}, m_task{[this, fn = std::move(fn)]() { m_ret = fn(); },
                        pre_tasks, scheduler} {}
  T &wait() {
    m_task.wait();
    return m_ret.value();
  }
};

class Scheduler {
  friend class Task;
  struct ThreadWrapper {
    ThreadWrapper(Scheduler &scheduler);
    std::thread m_thread;
    std::atomic_bool m_stop_flag;
  };

  std::queue<Task *> m_ready_tasks{};
  std::mutex m_ready_tasks_mutex{};
  std::condition_variable m_ready_tasks_cv{};

  std::vector<ThreadWrapper *> m_thread_pool{};

public:
  Scheduler(std::size_t executor_count);
  ~Scheduler();

private:
  void mark_task_ready(Task *task);
  Task *pop_ready_task();
};

} // namespace cpjview
