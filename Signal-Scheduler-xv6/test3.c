#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PROCS 3 // Number of processes to create

int
main(void)
{
  for (int i = 0; i < NUM_PROCS; i++) {
    int pid = custom_fork(1, 50); // start_later = 1, exec_time = 50 ticks

    if (pid < 0) {
      printf(1, "Failed to fork process %d\n", i);
      exit();
    } else if (pid == 0) {
      // Child process
      printf(1, "Child %d (PID: %d) started but should not run yet.\n", i, getpid());
      sleep(10);
      // Simulated work
      for (volatile int j = 0; j < 100000000; j++);

      exit();
    }
  }

  printf(1, "All child processes created with start_later flag set.\n");

  // Sleep to simulate some delay before scheduling starts
  sleep(400);

  printf(1, "Calling scheduler_start() to allow execution.\n");
  scheduler_start();

  // Wait for all child processes to finish
  for (int i = 0; i < NUM_PROCS; i++) {
    wait();
  }

  printf(1, "All child processes completed.\n");
  exit();
}
