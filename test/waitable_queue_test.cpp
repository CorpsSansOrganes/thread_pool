#include "waitable_queue.hpp" // EK::waitable_queue

#include <iostream>           // std::cout, std::endl
#include <thread>             // std::thread
#include <vector>             // std::vector

static int SmokeTest();
static int FailedTimeoutDeque();
static int SuccessfulTimeoutDeque();
static int OneProducerMultipleConsumers(int n);

template<typename T>
static void Producer(EK::WaitableQueue<T> &waitable_queue, int n);
template<typename T>
static void Consumer(EK::WaitableQueue<T> &waitable_queue, std::mutex& mutex, int& sum);

// Runner
int main() {
  int status = 0;
  
  status += SmokeTest();
  status += FailedTimeoutDeque();
  status += SuccessfulTimeoutDeque();
  status += OneProducerMultipleConsumers(5);

  if (0 == status) {
    std::cout << "SUCCESS: WaitableQueue" << std::endl;
  }
  return status;
}

// Tests
static int SmokeTest() {
  // Smoke test: if things don't crash and burn we're happy.
  EK::WaitableQueue<int> waitable_queue;
  return 0;
}

static int FailedTimeoutDeque() {
  // Attempting to deque from the waitable queue and failing.
  EK::WaitableQueue<int> waitable_queue;
  int res = 0;
  int status = (false != 
      waitable_queue.Deque(std::chrono::milliseconds(1), res));

  if (status) {
    std::cout << "FAILED: FailedTimeoutDeque" << std::endl;
    std::cout << "Expected Deque(timeout, outparam) to return false," << 
      " but true was returned instead." << std::endl;
  } 

  return status;
}

static int SuccessfulTimeoutDeque() {
  // Attempting to deque from the waitable queue and succeeding.
  EK::WaitableQueue<int> waitable_queue;
  int res = 0;

  waitable_queue.Enqueue(1234);
  int status = (true !=
      waitable_queue.Deque(std::chrono::milliseconds(1), res));

  if (status) {
    std::cout << "FAILED: SuccessfulTimeoutDeque" << std::endl;
    std::cout << "Expected Deque(timeout, outparam) to return true," << 
      " but false was returned instead." << std::endl;
  } 

  if (1234 != res) {
    std::cout << "FAILED: SuccessfulTimeoutDeque" << std::endl;
    std::cout << "Expected outparam to be equal 1234, but instead it is "
      << res << std::endl;
    ++status;
  }

  return status;
}

static int OneProducerMultipleConsumers(int n) {
  // One producer, multiple consumers scenrio.
  std::vector<std::thread> threads;

  // Create consumers & producer
  EK::WaitableQueue<int> waitable_queue;
  std::mutex mutex;
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    threads.emplace_back(std::thread(Consumer<int>, 
        std::ref(waitable_queue), std::ref(mutex), std::ref(sum)));
  }

  threads.emplace_back(std::thread(Producer<int>, std::ref(waitable_queue), n));

  // Join threads and check that sum is as expected.
  for (auto& t: threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  int expected_sum = (n * (n - 1)) / 2;
  if (sum != expected_sum) {
    std::cout << "FAILED: OneProducerMultipleConsumers " << std::endl;
    std::cout << "Expected sum to be " << expected_sum <<
      " but instead got " << sum << std::endl;
    return 1;
  }

  return 0;
}

// Utilities
template<typename T>
static void Producer(EK::WaitableQueue<T> &waitable_queue, int n) {
  // Produce values [0..n]
  for (int i = 0; i < n; ++i) {
    waitable_queue.Enqueue(i);
  }
}

template<typename T>
static void Consumer(EK::WaitableQueue<T> &waitable_queue,
    std::mutex& mutex,
    int& sum) {
  // Consume data from the waitable queue, and add it to sum.
  int value = waitable_queue.Deque();
  std::lock_guard<std::mutex> lock(mutex);
  sum += value;
}
