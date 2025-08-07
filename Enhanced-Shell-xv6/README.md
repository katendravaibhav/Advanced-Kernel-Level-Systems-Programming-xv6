# Enhanced Secure Shell for xv6

This project extends the base xv6 operating system by adding key features to improve user security, command tracking, and system control. It was developed as part of an Operating Systems course assignment at IIT Delhi.

## Features

### 1. Secure Login System
- Requires a predefined username and password to access the shell.
- Allows only three login attempts before denying access.
- Designed for simplicity and extensibility, making it easy to support encrypted credentials or file-based authentication in the future.

### 2. Command History
- Logs external command executions, including:
  - Process ID
  - Command name
  - Memory usage
- Limited to 64 entries to conserve system resources.
- A new system call `gethistory()` provides access to this log from user space.

### 3. Block/Unblock System Calls
- Enables users to selectively disable or enable system calls from within the shell.
- System call block state is maintained per process.
- Automatically clears restrictions in child processes after `exec()` to avoid unintended behavior.

### 4. Custom chmod Implementation
- Adds a new `chmod` system call to change file permissions.
- Modifies the on-disk inode using a union of bitfields to efficiently store permission bits.
- Maintains compatibility by preserving inode size.

## Technical Overview

- Language: C
- Platform: xv6 (teaching operating system)
- Key files modified:
  - `init.c` — login mechanism
  - `proc.c`, `proc.h` — command history, syscall blocking
  - `fs.h`, `sysproc.c` — chmod system call
  - `sh.c` — shell command integration

## Usage

- Boot the xv6 system with the enhanced kernel.
- Authenticate using the hardcoded credentials.
- Use shell commands:
  - `history` — display logged commands
  - `block <syscall_number>` — disable a system call
  - `unblock <syscall_number>` — re-enable a blocked system call
  - `chmod <file> <mode>` — change file permissions

## Future Work

- Support for secure and encrypted credentials
- Logging of blocked system call usage
- Extended command history (file-based or ring buffer)
- More advanced permission and access control mechanisms

## Author

Vaibhav Katendra  
IIT Delhi
---

Developed as part of Assignment 1 — Operating Systems Course
