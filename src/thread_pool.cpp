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

  void ThreadPool::Pause() const {
    // If thread pool isn't already paused:
    // Check how many threads there are. Then add PauseTask for each thread.
    // PauseTask contains a semaphore, which will block all threads.
    // Set is_paused flag to true to prevent multiple pauses.
  }

  void ThreadPool::Resume() const {
    // If thread pool is paused:
    // Check how many threads there are. Then Release the semaphore for each thread.
    // Set is_paused flag to flase to prevent multiple resumes.
  }

  /**-----------------*
   * PRIVATE FUNCTIONS *
   *------------------*/

} // end namespace EK
