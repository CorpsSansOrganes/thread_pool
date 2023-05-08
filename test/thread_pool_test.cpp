#include "semaphore.hpp"   // EK::Semaphore
#include "thread_pool.hpp" // EK::ThreadPool

#include <array>           // std::array
#include <chrono>          // std::chrono::milliseconds, std::chrono::seconds
#include <cstdlib>         // EXIT_FAILURE, EXIT_SUCCESS
#include <future>          // std::future_status
#include <iostream>        // std::cerr, std::endl
#include <mutex>           // std::mutex, std::unique_lock
#include <string>          // std::string
#include <thread>          // std::this_thread::sleep_for
#include <set>             // std::set

static int SmokeTest();
static int BasicUsageTest();
static int WaitForTasksTest();
static int PerfectForwardingTest();
static int PauseAndResumeTest();
static int MultiPauseAndMultiResumeTest();
static int SetNumThreadsTest();

static int CheckPerfectForwarding(std::string&& s);
static int CheckPerfectForwarding(const std::string& s);
static int ReturnArg(int i);
template <typename T>
static T Sum(T i, T j);

// Runner
int main() {
  int status = 0;

  status += SmokeTest();
  status += BasicUsageTest();
  status += WaitForTasksTest();
  status += PerfectForwardingTest();
  status += MultiPauseAndMultiResumeTest();
  status += PauseAndResumeTest();
  status += SetNumThreadsTest();

  if (0 == status) {
    std::cerr << "SUCCESS: Thread Pool" << std::endl;
  }

  return 0;
}

// Tests
static int SmokeTest() {
  EK::ThreadPool tp;
  return 0;
}

static int BasicUsageTest() {
  auto status = 0;
  EK::ThreadPool tp(4);

  // Send lambda with return value
  auto res1 = tp.Submit([](int x, int y) { return x + y; }, 1, 1);

  if (2 != res1.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected to get 2, but instead got " << res1.get() << std::endl;
    status += EXIT_FAILURE;
  }

  // Send lambda without return value
  int answer = 0;
  auto res2 = tp.Submit([](int* res) { *res = 42; }, &answer);
  res2.wait();
  if (42 != answer) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected answer to be 42, but instead it is " << answer << std::endl;
    status += EXIT_FAILURE;
  }

  // Use templative function
  // Note, it must be wrapped in lambda: can't pass function template as 
  // std::function argument.
  int i = 1;
  int j = 1;
  auto res3 = tp.Submit([](int i, int j) { return Sum(i, j); }, i, j);

  if (2 != res3.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected answer to be 2, but instead it is " << res3.get() << std::endl;
    status += EXIT_FAILURE;
  }

  auto res4 = tp.Submit([]() { return Sum(1, 1); });
  if (2 != res4.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected answer to be 2, but instead it is " << res4.get() << std::endl;
    status += EXIT_FAILURE;
  }

  // Sumbit a function 
  auto res5 = tp.Submit(ReturnArg, 0);
  if(0 != res5.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected function to return 0, but instead got " << res5.get() << std::endl;
    status += EXIT_FAILURE;
  }

  // Submit a functor
  class Functor {
  public:
    Functor() : was_called_(false)
      {}
    int operator()() const {
      this->was_called_ = true;
      return 42;
    }
    bool WasCalled() const {
      return this->was_called_;
    }

  private:
    mutable bool was_called_;
  };

  // Sumbit a functor object by reference, so that the object itself
  // is modified.
  auto func = Functor();
  auto res6 = tp.Submit(std::ref(func));

  res6.wait();
  if (true != func.WasCalled()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Functor state hasn't changed! expected was_called_ to be true." << std::endl;
    status += EXIT_FAILURE;
  }

  // Sumbit a functor object by value. Note that here the object itself 
  // will not be modified, as we pass a copy of it. Therefore, the functor's
  // state will not change, unlike before.
  auto func2 = Functor();
  auto res7 = tp.Submit(func2); 
  if (42 != res7.get() && false != func2.WasCalled()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Functor state changed!" << std:: endl;
    std::cerr << "Note: expected was_called_ to be false, it is " 
      << func2.WasCalled() << std::endl;
    std::cerr << "      expected return value to be 42, it is " << res7.get() << std::endl;
    status += EXIT_FAILURE;
  }

  return status;
}

static int WaitForTasksTest() {
  // Run 500 tasks. Make sure all tasks got executed before
  // the dtor was called.
  size_t num = 0;
  {
    std::mutex mutex;
    EK::ThreadPool tp;
    for (int i = 0; i < 500; ++i) {
      tp.Submit([&mutex, &num] { 
          std::unique_lock<std::mutex> lock (mutex);
          ++num;
          });
    }
  }

  if (500 != num) {
    std::cerr << "ERROR! WaitForTasksTest" << std::endl;
    std::cerr << "Expected num to be 500, instead got " << num << std::endl;
    std::cerr << "Thread pool was destructed before all tasks finished executing" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int PerfectForwardingTest() {
  // Making sure that when we call an overloaded function we still get perfect 
  // forwarding, that is the type category (l-value/r-value) is unaltered.
  std::string s = "I hope I land at CheckPerfectForwarding(std::string &&)!";
  EK::ThreadPool tp(1);
  if (1 != CheckPerfectForwarding(s)) {
    // Making sure the test works.
    return EXIT_FAILURE;
  }
  auto res = tp.Submit([&s] { return CheckPerfectForwarding(std::move(s)); });
  if (res.get()) {
    std::cerr << "ERROR: PerfectForwardingTest" << std::endl;
    std::cerr << "s was altered to an l-value reference" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int MultiPauseAndMultiResumeTest() {
  const size_t thread_count = 2;
  EK::ThreadPool tp(thread_count);

  // Making sure several pauses are prevented.
  tp.Pause();
  tp.Pause();
  auto res1 = tp.Submit([] { return 1; }); 
  tp.Resume();

  if (std::future_status::timeout ==
      res1.wait_for(std::chrono::milliseconds(100))) {
    std::cerr << "ERROR! PauseAndResumeTest" << std::endl;
    std::cerr << "Multiple pauses aren't prevented." << std::endl;
    return EXIT_FAILURE;
  }

  // Making sure several resumes are prevented.
  tp.Resume();
  tp.Resume();
  tp.Pause();
  auto res2 = tp.Submit([] { return 1; });

  if (std::future_status::ready == 
      res2.wait_for(std::chrono::milliseconds(100))) {
    std::cerr << "ERROR! PauseAndResumeTest" << std::endl;
    std::cerr << "Multiple resumes aren't prevented." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;

}

static int PauseAndResumeTest() {
  // Submit tasks to the thread pool, which count how many times they've
  // been run. Make sure that lines up with the expected amount when pausing
  // and resuming.

  auto status = 0;
  const size_t tasks_num = 10;
  const size_t thread_count = 2;
  EK::ThreadPool tp(thread_count);

  class CountFunctor {
  public:
    CountFunctor() : counter_(0) {}
    void operator()() {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      ++counter_;
    }
    int GetCounter() const {
      return counter_;
    }

  private:
    int counter_;
  };

  std::array<CountFunctor, tasks_num> tasks_arr;
  for (auto &t : tasks_arr) {
    tp.Submit(std::ref(t));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  tp.Pause();
  // Check that exactly thread_count tasks got performed.
  size_t actual_count = 0;
  for (const auto &t : tasks_arr) {
    actual_count += t.GetCounter();
  }
  size_t expected_count = thread_count;

  if (expected_count != actual_count) {
    std::cerr << "ERROR! PauseAndResumeTest" << std::endl;
    std::cerr << "After pausing, expected " << expected_count 
      << " tasks to be performed, but " << actual_count << " were." << std::endl;
    status += EXIT_FAILURE;
  }

  // Resume, make sure that all tasks got carried out.
  tp.Resume();
  tp.WaitForTasks();
  expected_count = tasks_num;
  actual_count = 0;
  for (const auto &t : tasks_arr) {
    actual_count += t.GetCounter();
  }

  if (expected_count != actual_count) {
    std::cerr << "ERROR! PauseAndReactual_counteTest" << std::endl;
    std::cerr << "After resuming, expected " << expected_count 
      << " tasks to be performed, but " << actual_count << " were." << std::endl;
    status += EXIT_FAILURE;
  }

  return status;
}

/** @brief To verify the number of threads inside the thread pool, submit twice as 
 * many tasks as there are threads.
 * To make sure each thread performs at least one task, after recording its id,
 * the thread is block until it is released by the main thread.
 * We then let the rest of the tasks to be executed, and make sure that the number
 * of IDs isn't higher or lower than what we expected.
 *
 * This test is fundemantally based on the pigeonhole principle. 
 * It is theoretically prone to a false negative, but it is highly unlikely.
 */
static int SetNumThreadsTest() {
  EK::ThreadPool tp;
  std::mutex mutex;
  std::mutex total_mutex;
  std::condition_variable total_cv;
  std::array<size_t, 3> test_cases = {2, 1, 3};

  for (auto thread_count : test_cases) {
    EK::Semaphore sem;
    size_t tasks_num = 2 * thread_count;
    size_t total_count = 0;
    std::set<std::thread::id> thread_ids;

    tp.SetNumThreads(thread_count);
    // Insert tasks to register id to the thread_ids set.
    auto register_id_task = [&total_count, &mutex, &total_cv, &sem, &thread_ids] {
          // Register id.
          {
            std::unique_lock<decltype(mutex)> lock(mutex);
            thread_ids.insert(std::this_thread::get_id());
            ++total_count;
          }
          total_cv.notify_one();        

          // Wait for main thread to unblock.
          sem.Acquire();
        };
    for (size_t i = 0; i < tasks_num; ++i) {
      tp.Submit(register_id_task);
    }

    // Main thread wait until thread_count different thread have performed tasks.
    // wait_for is used to prevent a deadlock if there are less threads than expected.
    std::unique_lock<decltype(total_mutex)> lock(total_mutex);
    total_cv.wait_for(lock,
        std::chrono::seconds(1),
        [&thread_count, &total_count] { return thread_count == total_count; });
    sem.Release(tasks_num);
    tp.WaitForTasks();

    if (thread_count != thread_ids.size()) {
      std::cerr << "ERROR: SetNumThreadsTest" << std::endl;
      std::cerr << "Expected " << thread_count << " threads, but instead there's "
        << thread_ids.size() << " threads in the thread pool." << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

// Utilities

template <typename T>
static T Sum(T i, T j) {
  return i + j;
}

static int ReturnArg(int i) {
  return i;
}

static int CheckPerfectForwarding(std::string&& s) {
  return 0;
  (void)s;
}

static int CheckPerfectForwarding(const std::string& s) {
  return 1;
  (void)s;
}
