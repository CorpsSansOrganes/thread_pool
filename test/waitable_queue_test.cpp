#include "waitable_queue.hpp" // EK::waitable_queue

#include <iostream>           // std::cout, std::endl

static int SmokeTest();

// Runner
int main() {
  int status = 0;
  
  status += SmokeTest();

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

// Utilities
