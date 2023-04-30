/**
 * @file: thread_pool.hpp 
 *
 * @description: Thread Pool is a design pattern offering an API
 * for distributing tasks among a group of worker threads.
 * It is used to simplify performing tasks concurrently.
 *
 * By providing a dedicated object for multithreaded operations, 
 * it reduces the overhead cost of creating and destorying threads.
 * It also alleviates the headace of managing synchronisation between threads.
 *
 * Key features of this implementation:
 * 1. Get values back from tasks using futures.
 * 2. Add or remove threads at runtime.
 *
 * @author: Eden Kellner, 29/04/2023.
 *
 */
#pragma once

#include "waitable_queue.hpp" // EK::WaitableQueue
#include <thread>             // std::thread
#include <cstddef>            // size_t
#include <vector>             // std::vector

namespace EK {
  class ThreadPool {
    public:
      ThreadPool(size_t totalThreadCount = DetermineThreadCount(0));
      ~ThreadPool();
      
      // Uncopyable
      ThreadPool(const ThreadPool&) = delete;
      ThreadPool& operator=(const ThreadPool&) = delete;

    private:
      size_t thread_count_;
      std::vector<std::thread> threads_;
      [[nodiscard]] static size_t DetermineThreadCount(size_t thread_count);
  };
} // end namespace EK
