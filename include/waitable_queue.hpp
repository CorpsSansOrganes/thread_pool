/**
 * @file waitable_queue.hpp 
 * @author Eden Kellner
 * @date 29/04/2023
 *
 * @brief Thread-safe queue, allowing for multiple threads 
 * to insert new items, remove existing items or wait until a new
 * item becomes available to consume.
 *
 */

#pragma once 

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <chrono>             // std::chrono::milliseconds
#include <queue>              // std::queue
#include <atomic>             // std::atomic_uint

namespace EK {

  /**
   * @brief Waitable, thread-safe queue class.
   *
   * @paramt T is the value type which the container holds.
   * @paramt Container is the container class used. Needs to support the 
   * following methods: push(), pop(), front(), empty().
   */
  template <class T, class Container = std::queue<T>>
  class WaitableQueue {
    public:
      /**
       * @brief Construct a waitable queue.
       */
      WaitableQueue();

      /**
       * @brief Inserts a new item into the queue.
       *
       * @param value - the item we wish to insert.
       */
      void Enqueue(T value);

      /**
       * @brief Removes an item from the queue.
       * The thread will be blocked, waiting until an item is available.
       *
       * @return An item removed from the queue. 
       */

      T Deque();

      /**
       * @brief Removes an item from the queue, with a timeout.
       *
       * @param timeout dictates how much time a thread will wait for an item to
       * become available. After timeout milliseconds the function will return.
       * @param outparam used to return the item acquired.
       *
       * @return True if an item has been acquired, and False otherwise.
       */
      bool Deque(std::chrono::milliseconds timeout, T& outparam);

      /**
       * @brief Indicates if the queue is empty.
       *
       * @return true if the queue is empty, false otherwise.
       */
      bool IsEmpty() const;

      // Uncopyable
      WaitableQueue(const WaitableQueue&) = delete;
      WaitableQueue& operator=(const WaitableQueue&) = delete;

      // Default dtor is sufficient.
      ~WaitableQueue() = default;

    private:
      Container queue_;
      mutable std::mutex mutex_;
      std::condition_variable cv_;
      std::atomic_uint counter_;
    };

  // --- Implementation ---
  template <class T, class Container>
  WaitableQueue<T, Container>::WaitableQueue() :
    queue_(), mutex_(), cv_(), counter_(0) {}
  
  template <class T, class Container>
  void WaitableQueue<T, Container>::Enqueue(T value) {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    queue_.push(value);
    ++counter_;
    cv_.notify_one();
  }

  template <class T, class Container>
  T WaitableQueue<T, Container>::Deque() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    cv_.wait(lock, [&]{ return counter_ > 0; });
    --counter_;
    
    auto value = queue_.front();
    queue_.pop();
    return value;
  }

  template <class T, class Container>
  bool WaitableQueue<T, Container>::Deque(std::chrono::milliseconds timeout,
      T& outparam) {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    auto no_timeout = cv_.wait_for(lock, timeout, [&]{ return counter_ > 0; });

    if (no_timeout) {
      --counter_;
      outparam = queue_.front();
      queue_.pop();
      return true;
    } else {
      return false;
    }
  }

  template <class T, class Container>
  bool WaitableQueue<T, Container>::IsEmpty() const {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    return counter_;
  }
} // end namespace EK
