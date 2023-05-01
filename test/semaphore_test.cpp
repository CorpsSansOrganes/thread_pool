#include "semaphore.hpp" // EK::Semaphore

#include <chrono>
#include <iostream>           // std::cerr, std::endl
#include <thread>             // std::thread

static int SmokeTest(); 
static int AcquireTest();
static int TryAcquireForTest();

// Runner
int main() {
  int status = 0;
  
  status += SmokeTest();
  status += AcquireTest();
  status += TryAcquireForTest();

  if (0 == status) {
    std::cerr << "SUCCESS: Semaphore" << std::endl;
  }
  return status;
}

// Tests
static int SmokeTest() {
  // Smoke test: if things don't crash and burn we're happy.
  EK::Semaphore sem1;
  if (0 != sem1.GetCount()) {
    std::cerr << "FAILED: SmokeTest" << std::endl;
    std::cerr << "Expected semaphore to be initialised with 0, but instead it is " <<
      "initialised with " << sem1.GetCount() << std::endl;
    EXIT_FAILURE;
  }
  EK::Semaphore sem2(3);
  if (3 != sem2.GetCount()) {
    std::cerr << "FAILED: SmokeTest" << std::endl;
    std::cerr << "Expected semaphore to be initialised with 3, but instead it is " <<
      "initialised with " << sem2.GetCount() << std::endl;
    EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static int AcquireTest() {
  // Checking simple signaling and synchronisation functionality.
  EK::Semaphore sem(0);
  int flag = 0;

  std::thread t([&]() {
      sem.Acquire();
      flag = 1;
  });

  // Check that t is blocked on semaphore until we release.
  if (0 != flag) {
      sem.Release();
      t.join();
      std::cerr << "FAILED: AcquireTest" << std::endl;
      std::cerr << "Thread was not blocked by semaphore at all." << std::endl;
      return EXIT_FAILURE;
  }

  // Check that t will be unblocked once we release.
  sem.Release();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  if (1 != flag) {
      sem.Release();
      if (t.joinable()) {
        t.join();
      }
      std::cerr << "FAILED: AcquireTest" << std::endl;
      std::cerr << "Expected flag to be 1, instead it is " << flag << std::endl;
      return EXIT_FAILURE;
  }

  if (t.joinable()) {
    t.join();
  }
  return EXIT_SUCCESS;
}
static int TryAcquireForTest() {
  EK::Semaphore sem(0);

  // Try to acquire from semaphore, make sure we fail as expected
  auto res = sem.TryAcquireFor(std::chrono::milliseconds(10));
  if (false != res) {
      std::cerr << "FAILED: TryAcquireForTest" << std::endl;
      std::cerr << "Expected a failed TryAcquireFor() call to return false, " <<
        "instead got true" << std::endl;
      return EXIT_FAILURE;
  }

  // Post to semaphore then try to acquire from it.
  // Make sure we succeed as expected.
  sem.Release();
  res = sem.TryAcquireFor(std::chrono::milliseconds(10));
  if (true != res) {
      std::cerr << "FAILED: TryAcquireForTest" << std::endl;
      std::cerr << "Expected a succesful TryAcquireFor() call to return true, " <<
        "instead got false" << std::endl;
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
