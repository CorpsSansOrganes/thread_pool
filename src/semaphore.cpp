#include "semaphore.hpp" // EK::Semaphore
#include <mutex>

namespace EK {
  Semaphore::Semaphore(size_t initial_count) :
    mutex_(), cv_(), counter_(initial_count) {}

  void Semaphore::Release() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    ++counter_;
    cv_.notify_one();
  }

  void Semaphore::Release(size_t n) {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    counter_ += n;
    cv_.notify_all();
  }

  void Semaphore::Acquire() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    cv_.wait(lock, [&]{ return counter_ > 0; });
    --counter_;
  }

  bool Semaphore::TryAcquireFor(std::chrono::milliseconds timeout) {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    bool no_timeout = cv_.wait_for(lock, timeout, [&] { return counter_ > 0; });

    if (no_timeout) {
      --counter_;
      return true;
    } else {
      return false;
    }
  }

  size_t Semaphore::GetCount() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    return counter_;
  }
} // end namespace EK
