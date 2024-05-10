#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <type_traits>
#include <vector>

namespace cpjview {

class Scheduler;

class Task {
  friend Scheduler;
  class Impl;
  std::unique_ptr<Impl> m_impl;

public:
  Task(std::uint8_t priority, std::function<void()> fn,
       std::vector<Task *> const &pre_tasks, Scheduler &scheduler);
  Task(Task &&other);
  Task &operator=(Task &&other);
  ~Task();
  void wait();
};

template <class T> class TaskWithRet {
  std::optional<T> m_ret;
  Task m_task;

public:
  TaskWithRet(std::uint8_t priority, std::function<T()> fn,
              std::vector<Task *> const &pre_tasks, Scheduler &scheduler)
      : m_ret{},
        m_task{priority, [this, fn = std::move(fn)]() { m_ret = fn(); },
               pre_tasks, scheduler} {}
  T &wait() {
    m_task.wait();
    return m_ret.value();
  }
  Task &get_task() { return m_task; }
};

template <class T>
using Promise = std::conditional_t<std::is_void_v<T>, Task, TaskWithRet<T>>;

class Scheduler {
  friend class Task;
  struct ThreadWrapper {
    ThreadWrapper(Scheduler &scheduler);
    std::thread m_thread;
    std::atomic_bool m_stop_flag;
  };

  class ReadyQueue;
  std::unique_ptr<ReadyQueue> m_ready_queue;
  std::vector<ThreadWrapper *> m_thread_pool{};

public:
  Scheduler(std::size_t executor_count);
  ~Scheduler();

private:
  void mark_task_ready(Task::Impl *task);
  Task::Impl *pop_ready_task(std::atomic_bool const &force_stop_flag);
};

} // namespace cpjview
