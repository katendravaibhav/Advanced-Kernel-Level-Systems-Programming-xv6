# Memory Monitoring and Adaptive Page Swapping in xv6

This project enhances the xv6 operating system by adding two key features:

1. A **Memory Printer**, which displays the number of memory pages allocated to each process.
2. An **Adaptive Page Swapping Mechanism**, which swaps pages between RAM and disk based on current memory pressure.

Both features were developed as part of an Operating Systems course assignment.

## Features

### 1. Memory Printer (Ctrl+I)
- Triggered by pressing `Ctrl+I`.
- Displays memory usage for each process (PID and number of pages in RAM).
- Implemented by modifying the keyboard interrupt handler (`console.c`) and adding a `print_memory_usage()` function.
- Tracks resident set size (`rss`) for each process via a new field in `struct proc`.

### 2. Page Swapping
- Adds a swap area between the superblock and log on disk.
- Implements disk-based storage of pages when physical memory is limited (PHYSTOP limited to 4MB).
- Manages swap slots using a new module `pageswap.c`.

#### Swap Slot Structure
- 800 slots, each for a 4KB page (8 disk blocks).
- Each slot stores:
  - Page permissions
  - Availability status

### 3. Page Replacement Strategy
- Two-step victim selection:
  1. **Process Selection**: Choose process with highest `rss`.
  2. **Page Selection**: Choose least recently accessed page (`PTE_A` not set).
- Implements a second-chance-like algorithm by clearing `PTE_A` bits and retrying if no victim is found.

### 4. Swapping Mechanisms
- **swapAndPageOut()**: Writes a page to disk, updates PTE.
- **swapPageIn()**: Handles page faults by reading swapped-out pages back into memory.
- Integrates with the xv6 page fault handler (`trap.c`).

### 5. Adaptive Swapping Policy
- Controlled via two tunable parameters: **α (ALPHA)** and **β (BETA)**.
- Behavior:
  - **α = 25**: Increases number of pages to swap by 25% after each trigger.
  - **β = 10**: Decreases the threshold for swapping by 10% after each trigger.
- Implements dynamic adjustment based on memory pressure:
  - When free pages fall below the threshold, `npages_to_swap` increases and `threshold` decreases.

### 6. Testing and Results
- Tested using a `memtest.c` program that allocates large amounts of memory.
- Output shows increasing swap aggressiveness as free memory drops.
- Confirms that adaptive behavior avoids process failure under memory pressure.

### Example Output
Current Threshold = 100, Swapping 4 pages
Current Threshold = 90, Swapping 5 pages
...
Current Threshold = 20, Swapping 100 pages
Memtest Passed


## Tunable Parameters

- **ALPHA (α):** Controls rate of swap aggressiveness increase.
  - Higher values = more pages swapped each time.
- **BETA (β):** Controls threshold decay.
  - Higher values = more frequent triggers.

Recommended values:
- `α = 25`, `β = 10` for balanced performance.

## Implementation Summary

### Modified Files
- `console.c` – Keyboard interrupt for Ctrl+I
- `proc.h` – Added `rss` field
- `pageswap.c` – Swapping logic and policies
- `memlayout.h` – Memory limit
- `trap.c` – Page fault handling

### New/Key Functions
- `print_memory_usage()` – Process memory display
- `swapAndPageOut()`, `swapPageIn()` – Swapping operations
- `findVictimProc()`, `findVictimPage()` – Page replacement logic
- `checkAndSwap()` – Adaptive policy manager

## Authors

- Vaibhav Katendra – 2024MCS2459  
- IIT Delhi
