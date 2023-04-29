#include "thread_pool.hpp" // EK::ThreadPool

#include <iostream>        // std::cout, std::endl

static int SmokeTest();

// Runner
int main() {
  int status = 0;

  status += SmokeTest();

  if (0 == status) {
    std::cout << "SUCCESS: Thread Pool" << std::endl;
  }

  return 0;
}

// Tests
static int SmokeTest() {
  EK::ThreadPool tp;
  return 0;
}

// Utilities
