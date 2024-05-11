#pragma once

#include <__type_traits/unwrap_ref.h>
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
  std::vector<std::shared_ptr<Task>> m_post_tasks{};
  std::mutex m_mutex{}; // lock order should be from pre to post
  std::condition_variable m_cv{};

public:
  ~Task() { wait(); }

  void wait();

  Task(std::uint8_t priority, std::function<void()> fn, std::size_t pre_count);

  static void
  sync_with_pre_tasks(std::shared_ptr<Task> self,
                      std::vector<std::shared_ptr<Task>> const &pre_tasks,
                      Scheduler &scheduler);

private:
  void run(Scheduler &scheduler);
  void post_run(Scheduler &scheduler);
  bool insert_post(std::shared_ptr<Task> post_task);
  static void update_finished_pre_count(std::shared_ptr<Task> &self,
                                        std::size_t finished_count,
                                        Scheduler &scheduler);
};

template <class T> class TaskWithRet : public Task {
  std::optional<T> m_ret;

public:
  TaskWithRet(std::uint8_t priority, std::function<T()> fn,
              std::size_t pre_count)
      : Task{priority, [this, fn = std::move(fn)]() { m_ret = fn(); },
             pre_count},
        m_ret{} {}
  T &wait_for_value() {
    Task::wait();
    return m_ret.value();
  }
};

template <class T> class Promise {
  using Type = std::conditional_t<std::is_void_v<T>, Task, TaskWithRet<T>>;

  std::shared_ptr<Type> m_task;

public:
  Promise(std::uint8_t priority, std::function<T()> fn,
          std::vector<std::shared_ptr<Task>> const &pre_tasks,
          Scheduler &scheduler)
      : m_task(
            std::make_shared<Type>(priority, std::move(fn), pre_tasks.size())) {
    Task::sync_with_pre_tasks(std::static_pointer_cast<Task>(m_task), pre_tasks,
                              scheduler);
  }

  std::shared_ptr<Type> get_task() { return m_task; }
};

class Scheduler {
  friend class Task;
  struct ThreadWrapper {
    ThreadWrapper(Scheduler &scheduler);
    std::thread m_thread;
    std::atomic_bool m_stop_flag;
  };

  class ReadyQueue;
  std::unique_ptr<ReadyQueue> m_ready_queue;
  std::vector<std::unique_ptr<ThreadWrapper>> m_thread_pool{};

public:
  Scheduler(std::size_t executor_count);
  ~Scheduler();

private:
  void mark_task_ready(std::shared_ptr<Task> task);
  std::shared_ptr<Task> pop_ready_task(std::atomic_bool const &force_stop_flag);
};

} // namespace cpjview
