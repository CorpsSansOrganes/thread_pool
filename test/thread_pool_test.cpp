#include "thread_pool.hpp" // EK::ThreadPool

#include <array>
#include <chrono>          // std::chrono::milliseconds, std::chrono::seconds
#include <cstdlib>         // EXIT_FAILURE, EXIT_SUCCESS
#include <future>          // std::future_status
#include <iostream>        // std::cerr, std::endl
#include <memory>          // make_unique
#include <mutex>           // std::mutex, std::unique_lock
#include <string>          // std::string
#include <thread>          // std::this_thread::sleep_for

static int SmokeTest();
static int BasicUsageTest();
static int WaitForTasksTest();
static int PerfectForwardingTest();

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
  EK::ThreadPool tp(4);

  // Send lambda with return value
  auto res1 = tp.Submit([](int x, int y) { return x + y; }, 1, 1);

  if (2 != res1.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected to get 2, but instead got " << res1.get() << std::endl;
    return EXIT_FAILURE;
  }

  // Send lambda without return value
  int answer = 0;
  auto res2 = tp.Submit([](int& res) { res = 42; }, answer);
  res2.wait();
  if (42 != answer) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected answer to be 42, but instead it is " << answer << std::endl;
    return EXIT_FAILURE;
  }

  // Use templative function
  // Note, it must be wrapped in lambda: can't pass function template as 
  // std::function argument.
  int i = 1;
  int j = 1;
  auto res3 = tp.Submit([](int i, int j) {return Sum(i, j); }, i, j);

  if (2 != res3.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected answer to be 2, but instead it is " << res3.get() << std::endl;
    return EXIT_FAILURE;
  }

  auto res4 = tp.Submit([]() {return Sum(1, 1); });
  if (2 != res3.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected answer to be 2, but instead it is " << res4.get() << std::endl;
    return EXIT_FAILURE;
  }

  // Sumbit a function 
  auto res5 = tp.Submit(ReturnArg, 0);
  if(0 != res5.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected function to return 0, but instead got " << res5.get() << std::endl;
    return EXIT_FAILURE;
  }

  // Submit a functor
  class Functor {
  public:
    Functor() : was_called_(false)
      {}
    void operator()() const {
      was_called_ = true;
    }
    bool WasCalled() const {
      return was_called_;
    }

  private:
    mutable bool was_called_;
  };

  auto func = Functor();
  auto res6 = tp.Submit(func);
  res6.wait();
  if (true != func.WasCalled()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Functor wasn't called! expected was_called_ to be true." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
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
  auto res = tp.Submit([s] { return CheckPerfectForwarding(std::move(s)); });
  if (res.get()) {
    std::cerr << "ERROR: PerfectForwardingTest" << std::endl;
    std::cerr << "s was altered to an l-value reference" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

static int PauseAndResumeTest() {
  const size_t thread_count = 2;
  const size_t tasks_num = 10;
  EK::ThreadPool tp(thread_count);

  // Making sure several pauses are prevented.
  tp.Pause();
  tp.Pause();
  auto res = tp.Submit([] { return 1; }); 
  tp.Resume();

  if (std::future_status::timeout ==
      res.wait_for(std::chrono::milliseconds(100))) {
    std::cerr << "ERROR! PauseAndResumeTest" << std::endl;
    std::cerr << "Multiple pauses aren't prevented." << std::endl;
    return EXIT_FAILURE;
  }

  // Making sure several resumes are prevented.
  tp.Resume();
  tp.Pause();
  res = tp.Submit([] { return 1; });

  if (std::future_status::ready == 
      res.wait_for(std::chrono::milliseconds(100))) {
    std::cerr << "ERROR! PauseAndResumeTest" << std::endl;
    std::cerr << "Multiple resumes aren't prevented." << std::endl;
    return EXIT_FAILURE;
  }

  // Submit tasks to the thread pool, which count how many times they've
  // been run. Make sure that lines up with the expected amount when pausing
  // and resuming.

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
    tp.Submit(t);
  }

  tp.Pause();
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Check that exactly tasks_num - thread_count tasks got performed.
  auto actual_count = 0;
  for (const auto &t : tasks_arr) {
    actual_count += t.GetCounter();
  }
  auto expected_count = tasks_num - thread_count;

  if (expected_count != actual_count) {
    std::cerr << "ERROR! PauseAndReactual_counteTest" << std::endl;
    std::cerr << "After pausing, expected " << expected_count 
      << " tasks to be performed, but " << actual_count << " were." << std::endl;
    return EXIT_FAILURE;
  }

  // Resume, make sure that all tasks got carried out.
  tp.Resume();
  expected_count = tasks_num;
  actual_count = 0;
  for (const auto &t : tasks_arr) {
    actual_count += t.GetCounter();
  }

  if (expected_count != actual_count) {
    std::cerr << "ERROR! PauseAndReactual_counteTest" << std::endl;
    std::cerr << "After resuming, expected " << expected_count 
      << " tasks to be performed, but " << actual_count << " were." << std::endl;
    return EXIT_FAILURE;
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
}

static int CheckPerfectForwarding(const std::string& s) {
  return 1;
}
