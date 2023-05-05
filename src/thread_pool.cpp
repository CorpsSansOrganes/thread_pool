#include "thread_pool.hpp" // EK::ThreadPool
#include <cmath>         // std::abs

namespace EK {
  /**-----------------*
   * PUBLIC FUNCTIONS *
   *------------------*/
  
  ThreadPool::ThreadPool(size_t thread_count) :
    thread_count_(DetermineThreadCount(thread_count))
  {
    // Create threads with thread handler
    CreateThreads(thread_count_);
  }

  ThreadPool::~ThreadPool() {
    WaitForTasks();
    SetThreadNum(0);
  }

  void ThreadPool::SetThreadNum(std::size_t num_threads) {
    size_t diff = std::abs(static_cast<long long>(num_threads - thread_count_));
    if (num_threads > thread_count_) {
      CreateThreads(diff);
    } else {
      RemoveThreads(diff);
    }
  }

  void ThreadPool::Pause() {
    if (is_paused_) {
      return;
    }

    for (auto i = 0; i < thread_count_; ++i) {
      this->Submit([this] {
          this->pause_sem_.Acquire();
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
    for (int i = 0; i < thread_count; ++i) {
      auto new_thread = std::thread(&ThreadPool::ServeTasks, this);
      threads_.emplace(new_thread.get_id(), new_thread);
    }
  }

  static size_t DetermineThreadCount(size_t thread_count) {
    // User specified number of threads.
    if (thread_count > 0) {
      return thread_count;
    }

    // Default: total number of hardware threads available.
    // If unable to detect, return 1.
    thread_count = std::thread::hardware_concurrency();
    return thread_count ? thread_count : 1;
  }

  void ThreadPool::ServeTasks() {
    auto id = std::this_thread::get_id();

    {
      std::unique_lock<decltype(mutex_)> lock(mutex_);
      should_run_[id] = true;
    }

    // Serve tasks
    while (should_run_[id]) {
      auto task = tasks_.Deque();
      waiting_cv_.notify_one();
      task();
    }

    // When terminating, add oneself to the joinable threads queue.
    joinable_threads_.Enqueue(id);
  }

  void ThreadPool::WaitForTasks() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    waiting_cv_.wait(lock, [this] { return tasks_.IsEmpty(); });
  }
} // end namespace EK
