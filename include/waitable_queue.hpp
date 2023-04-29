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

#include <mutex>              // std::mutex, std::scope_lock
#include <condition_variable> // std::condition_variable
#include <chrono>             // std::chrono::milliseconds
#include <queue>              // std::queue

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
     * @return True if the queue is empty, False otherwise.
     */
    bool IsEmpty() const;

    private:
    Container m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    };
}
