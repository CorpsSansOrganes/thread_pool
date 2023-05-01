#include "thread_pool.hpp" // EK::ThreadPool

namespace EK {
  ThreadPool::ThreadPool(size_t thread_count) {
  }

  ThreadPool::~ThreadPool() {
    WaitForTasks();
    SetThreadNum(0);
  }

} // end namespace EK
