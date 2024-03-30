#include "cpjview/utils/scheduler.hpp"
#include "spdlog/spdlog.h"
#include <mutex>
#include <thread>

namespace cpjview {

Task::Task(std::function<void()> fn, std::vector<Task *> const &pre_tasks,
           Scheduler &scheduler)
    : m_fn{std::move(fn)}, m_pre_count{pre_tasks.size()} {
  std::size_t finished_pre = 0;
  for (Task *pre : pre_tasks) {
    if (!pre->insert_post()) {
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
bool Task::insert_post() {
  std::lock_guard<std::mutex> lock{m_mutex};
  if (!m_is_finished) {
    m_post_tasks.push_back(this);
    return true;
  }
  return false;
}

void Task::update_finished_pre_count(std::size_t finished_count,
                                     Scheduler &scheduler) {
  std::lock_guard<std::mutex> lock{m_mutex};
  m_finished_pre_cont += finished_count;
  if (m_finished_pre_cont == m_pre_count) {
    m_is_finished = false;
    scheduler.mark_task_ready(this);
  }
}

Scheduler::ThreadWrapper::ThreadWrapper(Scheduler &scheduler)
    : m_thread{}, m_stop_flag{false} {
  m_thread = std::thread([this, &scheduler]() {
    while (m_stop_flag) {
      Task *task = scheduler.pop_ready_task();
      task->run(scheduler);
    }
  });
}

Scheduler::Scheduler(std::size_t executor_count) {
  for (std::size_t index = 0; index < executor_count; index++) {
    m_thread_pool.push_back(new ThreadWrapper(*this));
  }
}

Scheduler::~Scheduler() {
  for (ThreadWrapper *wrapper : m_thread_pool) {
    wrapper->m_stop_flag = true;
  }
  m_ready_tasks_cv.notify_all();
  for (ThreadWrapper *wrapper : m_thread_pool) {
    if (wrapper->m_thread.joinable()) {
      wrapper->m_thread.join();
    }
  }
  for (ThreadWrapper *wrapper : m_thread_pool) {
    delete wrapper;
  }
}

void Scheduler::mark_task_ready(Task *task) {
  {
    std::lock_guard<std::mutex> lock{m_ready_tasks_mutex};
    m_ready_tasks.push(task);
  }
  m_ready_tasks_cv.notify_one();
}

Task *Scheduler::pop_ready_task() {
  std::unique_lock<std::mutex> lock{m_ready_tasks_mutex};
  m_ready_tasks_cv.wait(lock,
                        [this]() -> bool { return !m_ready_tasks.empty(); });
  Task *task = m_ready_tasks.front();
  m_ready_tasks.pop();
  return task;
}

} // namespace cpjview
