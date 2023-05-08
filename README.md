# Thread Pool
## Overview
This repository contains C++11-compatible implementations for:

1. Thread Pool
2. Semaphore 
3. Waitable Queue. 

My goal was to make a robust, reliable thread pool while keeping it beginner-friendly and easy to understand.
Each component is well-documented and designed to make it as easy to use as possible.
Each component is accompanied by a test file which, in addition to ensuring correctness, provide multiple extended examples for how to use these components.

Here I'll provide an overview of each component, a technical documentation can be found inside each respective header file.

## Build
If you're using Linux, the makefile provided can be used to create executables for each module's respective test.
Simply run in your shell:
```SHELL
make all                  # Create all modules in release mode.
make mode=debug all       # Create all modules in debug mode.
```

You can also create a specific module:
```SHELL
make waitable_queue       # Create waitable_queue in release mode.
make mode=debug semaphore # Create semaphore in release mode.
```

## Table of Content
  * [1. Thread Pool](#1-thread-pool)
    + [What is it?](#what-is-it-)
    + [Features](#features)
    + [Usage](#usage)
  * [2. Semaphore](#2-semaphore)
    + [What is it?](#what-is-it--1)
    + [Features](#features-1)
    + [Usage](#usage-1)
  * [3. Waitable Queue](#3-waitable-queue)
    + [What is it?](#what-is-it--2)
    + [Features](#features-2)
    + [Usage](#usage-2)
    
## 1. Thread Pool 
### What is it?
A **thread pool** is an object which offers an API for distributing tasks among a group of threads, which then execute the submitted tasks
concurrently.

By having a dedicated object for multithreaded operations, a thread pool reduces the overhead cost of having to create and destroy threads.
Additionally, it alleviates the headache of managing synchronization between threads.

### Features
This particular implementations has the following key features:
1. Any callable (function, functor, lambda...) can be submitted as a task, alongside any number of arguments.
2. Tasks can return values asynchronously.
3. Add or remove threads from the thread pool at runtime.
4. Pause and resume the thread pool.

### Usage
```C++
// Create a new thread pool with maximum threads supported by the hardware.
ThreadPool tp; 

// Submit task with arguments.
auto sum_task = [](int x, int y) { return x + y; };
auto result = tp.Submit(sum_task, 2, 2);

// Get result back.
assert(result.get() == 4);
```

Additional examples can be found under `BasicUsageTest()` at `test/thread_pool_test.cpp`.

## 2. Semaphore
### What is it?
A **semaphore** is a signalling device, used for synchronization between threads.
Each semaphore has a counter. When a thread reaches a semaphore it checks its counter. If the counter is positive, the counter is decremented
and the thread passes. Otherwise, if the counter is zero, the thread will be blocked until the semaphore is incremented by another thread.

### Features
Portable, c++11 compatible implementation.

### Usage 
```C++
// Create a new semaphore with an initial counter 0.
Semaphore sem;

// Create a thread which will be blocked on the semaphore.
std::thread t([&] {
  sem.Acquire();
  std::cout << "Thread 2" << std::endl;
}

// Print & then unblock the thread.
std::cout << "Thread 1" << std::endl;
sem.Release();
```

Additional examples can be found under `test/semaphore_test.cpp`.

## 3. Waitable Queue
### What is it?
A **waitable queue** is a thread-safe data structure based on a queue. It allows for multiple threads to insert new items, remove existing
items or wait until a new item becomes available.

### Features 
1. Flexible: any container supporting push(), pop(), and front() to be used as the underlying data structure.
2. Lightweight: simply include `waitable_queue.hpp` to use it.

### Usage
```C++
// Create a new empty waitable queue.
WaitableQueue<int> waitable_queue;
std::mutex mutex;
int sum = 0;

// Create 3 threads, consuming values from the queue and summing them.
for (int i = 0; i < 3; ++i) {
  std::thread t([&] {
    while (true) {
      int value = waitable_queue.Dequeue();
      {
        std::unique_lock<std::mutex> lock(mutex);
        sum += value;
      }
    }
  });
}

// Produce values from main thread
for (int i = 0; i = 100; ++i) {
  waitable_queue.Enqueue(i);
}

// Sleep to let consumers finish, sum should be 4950 (= 100 * 99 / 2).
std::this_thread::sleep_for(std::chrono::seconds(1));
assert(sum == 4950);
```
