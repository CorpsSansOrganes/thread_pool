#include "thread_pool.hpp" // EK::ThreadPool
#include <cmath>           // std::abs

namespace EK {
  /**-----------------*
   * PUBLIC FUNCTIONS *
   *------------------*/
  
  ThreadPool::ThreadPool(size_t thread_count) :
    thread_count_(ThreadPool::DetermineThreadCount(thread_count)),
    threads_(), should_run_(), tasks_(), joinable_threads_(), mutex_(),
    is_paused_(false), pause_sem_(), waiting_cv_() {
    CreateThreads(thread_count_);
  }

  ThreadPool::~ThreadPool() {
    WaitForTasks();
    SetNumThreads(0);
  }

  void ThreadPool::SetNumThreads(std::size_t num_threads) {
    size_t diff = std::abs(static_cast<long long>(num_threads - thread_count_));
    if (num_threads > thread_count_) {
      CreateThreads(diff);
    } else {
      RemoveThreads(diff);
    }
    thread_count_ = num_threads;
  }

  void ThreadPool::Pause() {
    if (is_paused_) {
      return;
    }

    for (size_t i = 0; i < thread_count_; ++i) {
      Submit([this] {
          pause_sem_.Acquire();
        });
    }
    is_paused_ = true;
  }

  void ThreadPool::Resume() {
    if (!is_paused_) {
      return;
    }

    pause_sem_.Release(thread_count_);
    is_paused_ = false;
  }

  /**------------------*
   * PRIVATE FUNCTIONS *
   *-------------------*/

  void ThreadPool::CreateThreads(size_t thread_count) {
    for (size_t i = 0; i < thread_count; ++i) {
      auto new_thread = std::thread(&ThreadPool::ServeTasks, this);
      threads_.emplace(new_thread.get_id(), std::move(new_thread));
    }
  }

  size_t ThreadPool::DetermineThreadCount(size_t thread_count) {
    // User specified number of threads.
    if (thread_count > 0) {
      return thread_count;
    }

    // Default: total number of hardware threads available.
    // If unable to detect, return 1.
    thread_count = std::thread::hardware_concurrency();
    return thread_count ? thread_count : 1;
  }

  void ThreadPool::RemoveThreads(size_t thread_count) {
    // Each threads receives a task to terminate itself.
    for (size_t i = 0; i < thread_count; ++i) {
      Submit([this] { 
            std::unique_lock<decltype(mutex_)> lock(mutex_);
            should_run_[std::this_thread::get_id()] = false;
          });
    }

    // Join back threads that terminated.
    for (size_t i = 0; i < thread_count; ++i) { 
      auto id = joinable_threads_.Dequeue();
      auto terminated_thread = std::move(threads_[id]);
      threads_.erase(id);
      if (terminated_thread.joinable()) {
        terminated_thread.join();
      }
    }
  }

  void ThreadPool::ServeTasks() {
    auto id = std::this_thread::get_id();
    {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      should_run_[id] = true;
    }

    while (should_run_[id]) {
      auto task = tasks_.Dequeue();
      waiting_cv_.notify_one();
      task();
    }

    joinable_threads_.Enqueue(id);
  }

  void ThreadPool::WaitForTasks() {
    Resume();
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    waiting_cv_.wait(lock, [this] { return tasks_.IsEmpty(); });
  }
} // end namespace EK
