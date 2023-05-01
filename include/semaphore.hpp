/**
 * @file: semaphore.hpp 
 *
 * @description: C++11-compatible semaphore implementation.
 *
 * Semaphore is a signalling device, used for synchronisation between
 * threads, as it restricts access to critical code sections until a signal 
 * is received.
 *
 * The centre piece of a semaphore is a counter.
 * When a thread wants to pass the semaphore, it checks the counter.
 * 1. If the counter is positive, it decrements it and passes.
 * 2. If the counter is zero, the thread will be blocked until the semaphore is 
 *    incremented by another thread.
 *
 * @author: Eden Kellner, 01/05/2023.
 *
 */

#pragma once 

#include <chrono>             // std::chrono::milliseconds
#include <mutex>              // std::mutex
#include <condition_variable> // std::condition_variable

namespace EK {

  /**
   * @description: C++11-compatible semaphore.
   */
  class Semaphore {
    public:
      /**
       * @description Construct a sempahore.
       *
       * @param initial_count The value the semaphore's counter 
       * should be initialised with.
       */
      explicit Semaphore(size_t initial_count = 0);

      /**
       * @description Increment the counter and signal waiting threads.
       */
      void Release();

      /**
       * @description If the counter is positive, decrement it and pass.
       * Otherwise, wait until the sempahore is released by another thread.
       */
      void Acquire();

      /**
       * @description If the counter is positive, decrement it and pass.
       * Otherwise, wait until the semaphore is released by another thread 
       * or until timeout duration has been exceeded.
       *
       * @param timeout The maximum duration a thread will wait on the semaphore.
       *
       * @return true if decremented the counter, otherwise, if exceeded timeout, false.
       */
      bool TryAcquireFor(std::chrono::milliseconds timeout);

      /**
       * @description Get current counter value
       *
       * @return the value of the counter, currently.
       */
      size_t GetCount();

      // Uncopyable
      Semaphore(const Semaphore&) = delete;
      Semaphore& operator=(const Semaphore&) = delete;

      // Unmovable (because of std::mutex)
      Semaphore(Semaphore&&) = delete;
      Semaphore& operator=(Semaphore&&) = delete;

      // Default dtor is sufficient.
      ~Semaphore() = default;

    private:
      std::mutex mutex_;
      std::condition_variable cv_;
      size_t counter_;
    };
} // end namespace EK
