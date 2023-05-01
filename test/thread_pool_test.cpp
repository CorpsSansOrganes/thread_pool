#include "thread_pool.hpp" // EK::ThreadPool

#include <cstdlib>         // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>        // std::cerr, std::endl
#include <string>          // std::string

static int SmokeTest();
static int BasicUsageTest();
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

  // Simply call a function without any lambdas
  auto res5 = tp.Submit(ReturnArg, 0);
  if(0 != res5.get()) {
    std::cerr << "ERROR: BasicUsageTest" << std::endl;
    std::cerr << "Expected function to return 0, but instead got " << res5.get() << std::endl;
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
