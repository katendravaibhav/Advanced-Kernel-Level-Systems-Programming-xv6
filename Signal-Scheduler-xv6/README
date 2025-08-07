# Signal Handling and Scheduler Metrics Extension in xv6

This project enhances the xv6 operating system by implementing custom signal handling mechanisms tied to keyboard input and improving the process scheduler to support execution control and performance tracking.

## Summary of Features

### 1. Signal Handling via Keyboard Shortcuts

The keyboard driver is modified to detect the following combinations:

- **Ctrl+C (SIGINT)**  
  Terminates all user processes (PID > 2). Upon termination, the following metrics are printed:
  - Turnaround Time (TAT)
  - Waiting Time (WT)
  - Response Time (RT)
  - Number of Context Switches (#CS)

- **Ctrl+B (SIGBG)**  
  Suspends all eligible processes by setting a suspension flag.

- **Ctrl+F (SIGFG)**  
  Resumes suspended processes by clearing the suspension flag.

- **Ctrl+G (SIGCUSTOM)**  
  Triggers a user-defined signal handler if registered via a new `signal()` system call.

### 2. Scheduler Enhancements

#### a. `custom_fork(start_later, exec_time)`
- Extends the traditional fork.
- If `start_later` is set, the process will not run until `scheduler_start()` is called.
- `exec_time` restricts how long the process can run.

#### b. `scheduler_start()`
- Enables all processes that were started with `start_later = 1`.

#### c. Metrics Tracking
The scheduler tracks and reports:
- **Turnaround Time (TAT)** – Total lifetime of the process.
- **Waiting Time (WT)** – Time spent in the RUNNABLE state.
- **Response Time (RT)** – Time from creation to first scheduling.
- **Context Switches (#CS)** – Number of preemptions.

### 3. Priority Boosting with α and β

Dynamic priority is calculated as:

π(t) = π(0) - α * CPU_time + β * Wait_time


- Higher **α** penalizes CPU-intensive processes.
- Higher **β** favors processes that have waited longer.

### 4. Experimental Results

Test outcomes demonstrate:
- Low response time for all processes (RT = 1).
- Increasing TAT and WT across child processes.
- High context switches under CPU load.
- Sensitivity to α and β values for balancing fairness and throughput.

### 5. Modifications Overview

#### a. Data Structure Updates (in `proc`)
- `suspended` flag
- `sighandler` function pointer
- `waiting_time`, `total_ticks`
- `creation_time`, `end_time`, `response_time`
- `context_switches`

#### b. Key Modified Functions
- `consoleintr()` – Signal detection
- `scheduler()` – Enhanced scheduling logic
- `signal()` – Registering signal handlers
- `custom_fork()` – Fork with deferred scheduling
- `scheduler_start()` – Activates deferred processes

## Usage Instructions

1. Build and boot the modified xv6 kernel.
2. Press:
   - Ctrl+C to terminate user processes
   - Ctrl+B to suspend
   - Ctrl+F to resume
   - Ctrl+G to invoke a custom handler
3. Use `custom_fork()` with `start_later = 1` and `exec_time` to control execution.
4. Call `scheduler_start()` to start deferred processes.
5. View metrics after process termination.

## Authors

- Vaibhav Katendra
- IIT Delhi 
