# Extended xv6 Operating System

This repository provides a modular set of enhancements to the xv6 teaching operating system, focusing on improving system usability, process control, memory management, and performance tracking.

Each enhancement is self-contained within its own subdirectory and can be integrated individually or collectively depending on the desired system capabilities.

## Repository Structure

### 1. [`Enhanced-Shell-xv6`](./Enhanced-Shell-xv6)
Enhances the xv6 shell with the following features:

- **Secure Login System**  
  Authenticates users using predefined credentials with limited login attempts.

- **Command History Logging**  
  Records external commands along with process ID, name, and memory usage.

- **System Call Blocking**  
  Enables selective blocking/unblocking of system calls from within the shell.

- **Custom `chmod` Command**  
  Implements a system call to change file permissions by directly modifying inode metadata.

### 2. [`Signal-Scheduler-xv6`](./Signal-Scheduler-xv6)
Introduces real-time signal handling and advanced scheduler features:

- **Keyboard-triggered Signal Handling**  
  - `Ctrl+C`: Terminate user processes  
  - `Ctrl+B`: Suspend processes  
  - `Ctrl+F`: Resume suspended processes  
  - `Ctrl+G`: Invoke a user-defined signal handler

- **Enhanced Scheduler**  
  - Custom `fork()` with deferred execution and runtime limits  
  - `scheduler_start()` to activate deferred processes  
  - Metrics reporting: Turnaround Time, Waiting Time, Response Time, and Context Switches

- **Dynamic Priority Boosting**  
  Scheduler adapts process priorities using tunable parameters (α and β) based on CPU usage and wait time.

### 3. [`Memory-Swap-xv6`](./Memory-Swap-xv6)
Extends xv6's memory management with monitoring and adaptive swapping:

- **Memory Printer (Ctrl+I)**  
  Displays page count (RSS) for active processes.

- **Page Swapping Mechanism**  
  Swaps pages between RAM and disk using a disk-backed swap area.

- **Adaptive Page Replacement Policy**  
  Adjusts swapping threshold and page count dynamically in response to memory pressure using α and β tuning parameters.

## How to Use

Each subdirectory contains:

- Source code modifications
- Documentation of implementation details
- Instructions for integration and testing

To build and run a module:
1. Navigate into the desired subdirectory.
2. Integrate changes into your xv6 source tree (or use the provided modified version).
3. Compile and run xv6 in QEMU or your chosen environment.

## Compatibility

- All modules are based on the MIT xv6 (x86) public version.
- Tested in standard QEMU-based setups on Unix/Linux systems.
- Can be combined with care to create a unified extended xv6 kernel.

## License

This project builds on the open-source xv6 educational OS from MIT. Refer to the `LICENSE` file in each module for usage and distribution details.
