/**
 * @file: waitable_queue.hpp 
 *
 * @description: Thread-safe queue, allowing for multiple threads 
 * to insert new items, remove existing items or wait until a new
 * item becomes available to consume.
 *
 * @author: Eden Kellner, 29/04/2023.
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
   * @description: Waitable, thread-safe queue class.
   *
   * @paramt T is the value type which the container holds.
   * @paramt Container is the container class used. Needs to support the 
   * following methods: push(), pop(), front(), empty().
   */
  template <class T, class Container = std::queue<T>>
  class WaitableQueue {
    public:
      /**
       * @description: Construct a waitable queue.
       */
      WaitableQueue();

      /**
       * @description: Inserts a new item into the queue.
       *
       * @param value - the item we wish to insert.
       */
      void Enqueue(T value);

      /**
       * @description: Removes an item from the queue.
       * The thread will be blocked, waiting until an item is available.
       *
       * @return An item removed from the queue. 
       */
      T Deque();

      /**
       * @description: Removes an item from the queue, with a timeout.
       *
       * @param timeout dictates how much time a thread will wait for an item to
       * become available. After timeout milliseconds the function will return.
       *
       * @param outparam used to return the item acquired.
       *
       * @return True if an item has been acquired, and False otherwise.
       */
      bool Deque(std::chrono::milliseconds timeout, T& outparam);

      /**
       * @description: Indicates if the queue is empty.
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
      Container m_queue;
      mutable std::mutex m_mutex;
      std::condition_variable m_cv;
      std::atomic_uint m_counter;
    };

  // --- Implementation ---
  template <class T, class Container>
  WaitableQueue<T, Container>::WaitableQueue() :
    m_queue(), m_mutex(), m_cv(), m_counter(0) {}
  
  template <class T, class Container>
  void WaitableQueue<T, Container>::Enqueue(T value) {
    std::unique_lock<decltype(m_mutex)> lock(m_mutex);
    m_queue.push(value);
    ++m_counter;
    m_cv.notify_one();
  }

  template <class T, class Container>
  T WaitableQueue<T, Container>::Deque() {
    std::unique_lock<decltype(m_mutex)> lock(m_mutex);
    m_cv.wait(lock, [&]{ return m_counter > 0; });
    --m_counter;
    
    auto value = m_queue.front();
    m_queue.pop();

    return value;
  }

  template <class T, class Container>
  bool WaitableQueue<T, Container>::Deque(std::chrono::milliseconds timeout,
      T& outparam) {
    std::unique_lock<decltype(m_mutex)> lock(m_mutex);

    // Timeout
    if (!m_cv.wait_for(lock, timeout, [&]{ return m_counter > 0; })) {
      return false;
    }

    // No timeout
    --m_counter;
    outparam = m_queue.front();
    m_queue.pop();

    return true;
  }

  template <class T, class Container>
  bool WaitableQueue<T, Container>::IsEmpty() const {
    std::unique_lock<decltype(m_mutex)> lock(m_mutex);
    return m_counter;
  }

} // end namespace EK
