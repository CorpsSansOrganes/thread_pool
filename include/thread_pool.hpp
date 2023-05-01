/**
 * @file thread_pool.hpp 
 * @author Eden Kellner
 * @date 29/04/2023
 *
 * @brief Thread Pool is a design pattern offering an API
 * for distributing tasks among a group of worker threads.
 * It is used to simplify performing tasks concurrently.
 *
 * By providing a dedicated object for multithreaded operations, 
 * it reduces the overhead cost of creating and destorying threads.
 * It also alleviates the headace of managing synchronisation between threads.
 *
 * Additional features of this implementation:
 * 1. Supporting asynchronous return values from tasks.
 * 2. Add or remove threads at runtime.
 * 3. Pause and unpause the thread pool.
 */

#pragma once

#include "waitable_queue.hpp" // EK::WaitableQueue
#include "semaphore.hpp"      // EK::Semaphore
                              
#include <condition_variable> // std::condition_variable
#include <exception>          // std::current_exception
#include <thread>             // std::thread
#include <cstddef>            // size_t
#include <future>             // std::future
#include <map>                // std::map
#include <functional>         // std::bind
#include <utility>            // std::forward

namespace EK {
  class ThreadPool {
    public:
      /**
       * @brief Constructs a new thread pool. By default the number of threads
       * created is the total number of hardware threads avaiable.
       *
       * @param thread_count determines how many worker threads will be 
       * created initially.
       */
      ThreadPool(size_t thread_count = DetermineThreadCount(0));

      /**
       * @brief Destructs the thread pool.
       * Waiting for all tasks to complete before deallocating resources.
       */
      ~ThreadPool();

      /**
       * @brief Submit a new task to be executed by the thread pool.
       * The task can be any callable object (function, lambda etc).
       *
       * @tparam F The callable type (e.g. std::function<int()>).
       * @tparam Args The types of the zero or more arguments passed to the function.
       * @param task The callable to submit.
       * @param args The zero or more arguments passed to task 
       *
       * @return A future from which the return value of task can be retrieved.
       * If task has no return value, you would get std::future<void> which can 
       * be used to wait for the task to finish.
       */
      template <typename F, typename... Args>
      auto Submit(F&& task, Args&&... args) ->
        std::future<decltype(task(std::forward<Args>(args)...))>;

      /**
       * @brief Set the number of threads at runtime. 
       * Additional threads can be added, or threads can be removed.
       *
       * @param num_threads The number of worker threads the thread pool 
       * should posses.
       */
      void SetThreadNum(std::size_t num_threads);

      /**
       * @brief Pauses any additional tasks from executing.
       * Tasks that are currently executed won't be passed.
       */
      void Pause();

      /**
       * @brief Resumes task execution after being pauses.
       */
      void Resume();
      
      // Uncopyable
      ThreadPool(const ThreadPool&) = delete;
      ThreadPool& operator=(const ThreadPool&) = delete;

    private:
      size_t thread_count_;
      std::map<std::thread::id, bool> threads_;
      WaitableQueue<std::thread::id> joinable_threads_;
      WaitableQueue<std::function<void()>> tasks_;
      std::mutex tasks_mutex_;
      std::condition_variable cv_new_task_;
      size_t tasks_available_;

      [[nodiscard]] static size_t DetermineThreadCount(size_t thread_count);
      void CreateThreads();
  };

  // --- implementation ---
  template <typename F, typename... Args>
    auto ThreadPool::Submit(F&& task, Args&&... args) ->
    std::future<decltype(task(std::forward<Args>(args)...))> {

      // Shorthand for return type of calling task with args.
      using return_t = decltype(task(std::forward<Args>(args)...));

      // Creating callable and promise
      std::function<return_t()> task_function = std::bind(std::forward<F>(task),
          std::forward<Args>(args)...);
      auto task_promise = std::make_shared<std::promise<return_t>>();
      

      // Determine whether task(args...) returns or not, and add an appropriate
      // task to the waitable queue.
      if (std::is_void<return_t>::value) {
        tasks_.Enqueue(
            [task_function, task_promise] {
              try {
                task_function();
                task_promise->set_value();
              } catch (...) {
                task_promise->set_exception(std::current_exception());
              }
            });
      } else {
        tasks_.Enqueue(
            [task_function, task_promise] {
              try {
                task_promise->set_value(task_function());
              } catch (...) {
                task_promise->set_exception(std::current_exception());
              }
            });
      }

      return task_promise->get_future();
    }
} // end namespace EK
