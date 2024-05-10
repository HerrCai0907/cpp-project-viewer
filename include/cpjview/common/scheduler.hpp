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

  std::size_t m_id{};
  std::uint8_t m_priority;
  std::function<void()> m_fn;
  std::size_t m_pre_count;
  std::size_t m_finished_pre_cont{0u};
  bool m_is_finished{false};
  std::vector<Task *> m_post_tasks{};
  std::mutex m_mutex{}; // lock order should be from pre to post
  std::condition_variable m_cv{};

public:
  Task(std::uint8_t priority, std::function<void()> fn,
       std::vector<Task *> const &pre_tasks, Scheduler &scheduler);
  void wait();

private:
  void run(Scheduler &scheduler);
  void post_run(Scheduler &scheduler);
  bool insert_post(Task *post_task);
  void update_finished_pre_count(std::size_t finished_count,
                                 Scheduler &scheduler);
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
using Promise = std::unique_ptr<
    std::conditional_t<std::is_void_v<T>, Task, TaskWithRet<T>>>;

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
  void mark_task_ready(Task *task);
  Task *pop_ready_task(std::atomic_bool const &force_stop_flag);
};

} // namespace cpjview
