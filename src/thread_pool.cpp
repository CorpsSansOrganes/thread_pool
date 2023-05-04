#include "thread_pool.hpp" // EK::ThreadPool

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
    // Determine if threads should be added or reduced.
    // Then either call CreateThreads(...) or RemoveThreads(...) with the
    // appropriate number.
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

  /**-----------------*
   * PRIVATE FUNCTIONS *
   *------------------*/

} // end namespace EK
