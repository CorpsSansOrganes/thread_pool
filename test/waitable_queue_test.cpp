#include "waitable_queue.hpp" // EK::WaitableQueue

#include <cstdlib>            // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>           // std::cerr, std::endl
#include <thread>             // std::thread
#include <vector>             // std::vector

static int SmokeTest();
static int FailedTimeoutDequeue();
static int SuccessfulTimeoutDequeue();
static int OneProducerMultipleConsumers(int n);
static int EmptyTest();
static int SizeTest();

template<typename T>
static void Producer(EK::WaitableQueue<T> &waitable_queue, int n);
template<typename T>
static void Consumer(EK::WaitableQueue<T> &waitable_queue, std::mutex& mutex, int& sum);

// Runner
int main() {
  int status = 0;
  
  status += SmokeTest();
  status += FailedTimeoutDequeue();
  status += SuccessfulTimeoutDequeue();
  status += OneProducerMultipleConsumers(5);
  status += EmptyTest();
  status += SizeTest();

  if (EXIT_SUCCESS == status) {
    std::cerr << "SUCCESS: WaitableQueue" << std::endl;
  }
  return status;
}

// Tests
static int SmokeTest() {
  // Smoke test: if things don't crash and burn we're happy.
  EK::WaitableQueue<int> waitable_queue;
  return EXIT_SUCCESS;
}

static int EmptyTest() {
  int status = 0;
  EK::WaitableQueue<int> waitable_queue;
  if (true != waitable_queue.IsEmpty()) {
    std::cout << "ERROR: EmptyTest" << std::endl;
    std::cout << "IsEmpty() for newly created waitable queue retuned false!" << std::endl;
    status += EXIT_FAILURE;
  }

  waitable_queue.Enqueue(1);
  if (false != waitable_queue.IsEmpty()) {
    std::cout << "ERROR: EmptyTest" << std::endl;
    std::cout << "IsEmpty() for queue with 1 element retuned true!" << std::endl;
    status += EXIT_FAILURE;
  }

  waitable_queue.Dequeue();
  if (true != waitable_queue.IsEmpty()) {
    std::cout << "ERROR: EmptyTest" << std::endl;
    std::cout << "IsEmpty() for queue that's been emptied retuned false!" << std::endl;
    status += EXIT_FAILURE;
  }

  return status;
}

static int SizeTest() {
  int status = 0;
  EK::WaitableQueue<int> waitable_queue;
  if (0 != waitable_queue.Size()) {
    std::cout << "ERROR: SizeTest" << std::endl;
    std::cout << "Size() for newly created waitable queue retuned " 
      << waitable_queue.Size() << std::endl;
    status += EXIT_FAILURE;
  }

  waitable_queue.Enqueue(1);
  if (1 != waitable_queue.Size()) {
    std::cout << "ERROR: SizeTest" << std::endl;
    std::cout << "Size() for queue with 1 element retuned " << waitable_queue.Size() << std::endl;
    status += EXIT_FAILURE;
  }

  waitable_queue.Dequeue();
  if (0 != waitable_queue.Size()) {
    std::cout << "ERROR: SizeTest" << std::endl;
    std::cout << "Size() for queue that's been emptied retuned " 
      << waitable_queue.Size() << std::endl;
    status += EXIT_FAILURE;
  }

  return status;
}
static int FailedTimeoutDequeue() {
  // Attempting to deque from the waitable queue and failing.
  EK::WaitableQueue<int> waitable_queue;
  int res = 0;
  int status = (false != 
      waitable_queue.Dequeue(std::chrono::milliseconds(1), res));

  if (status) {
    std::cerr << "FAILED: FailedTimeoutDequeue" << std::endl;
    std::cerr << "Expected Dequeue(timeout, outparam) to return false," << 
      " but true was returned instead." << std::endl;
  } 

  return status;
}

static int SuccessfulTimeoutDequeue() {
  // Attempting to deque from the waitable queue and succeeding.
  EK::WaitableQueue<int> waitable_queue;
  int res = 0;

  waitable_queue.Enqueue(1234);
  int status = (true !=
      waitable_queue.Dequeue(std::chrono::milliseconds(1), res));

  if (status) {
    std::cerr << "FAILED: SuccessfulTimeoutDequeue" << std::endl;
    std::cerr << "Expected Dequeue(timeout, outparam) to return true," << 
      " but false was returned instead." << std::endl;
  } 

  if (1234 != res) {
    std::cerr << "FAILED: SuccessfulTimeoutDequeue" << std::endl;
    std::cerr << "Expected outparam to be equal 1234, but instead it is "
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
    std::cerr << "FAILED: OneProducerMultipleConsumers " << std::endl;
    std::cerr << "Expected sum to be " << expected_sum <<
      " but instead got " << sum << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
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
  int value = waitable_queue.Dequeue();
  std::lock_guard<std::mutex> lock(mutex);
  sum += value;
}
