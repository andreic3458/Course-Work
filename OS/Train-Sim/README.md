# Multi-Threaded Train Simulation

A concurrent train scheduling simulation written in C for CS3305 
(Operating Systems) at Western University.

## Overview

Simulates multiple trains loading and crossing a shared track section,
demonstrating core concurrency concepts in C.

## Implementation Details

- Each train runs as an independent POSIX thread
- 6 mutexes control access to shared track resources
- Conditional signaling coordinates train state transitions
  (loading → waiting → crossing)
- Enforces priority-based scheduling when multiple trains compete
  for track access

## How to Run

```bash
gcc train.c -o train -lpthread
./train
```

## Skills Demonstrated

- POSIX thread management
- Mutex synchronization and deadlock prevention
- Concurrent resource scheduling in C