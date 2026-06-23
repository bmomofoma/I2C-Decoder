# Custom Bare-Metal RTOS/Scheduler

## Overview

A lightweight real-time operating system kernel that manages multiple tasks running on a single processor. Tasks take turns executing (context switching), and the scheduler decides which task runs when. Built from scratch without using existing RTOS libraries like FreeRTOS.

---

## Core Concepts

### Tasks
Independent execution contexts. Each task has its own stack, registers, and program counter. Tasks run sequentially but appear to run in parallel due to rapid context switching. Each task needs a state (running, ready, blocked, suspended) and a priority level.

### Scheduler
Decides which task gets CPU time. Runs periodically (via timer interrupt or voluntary yield). Common strategies: round-robin (all tasks equal time), priority-based (higher priority runs first), or hybrid. Your scheduler should support at least 4-8 tasks.

### Context Switching
When a task yields or timer interrupt fires, save that task's registers and stack pointer, load the next task's registers and stack pointer, jump to its program counter. The CPU resumes executing the new task exactly where it left off.

### Stack Management
Each task needs its own stack in memory. When task A switches to task B, A's local variables are preserved on A's stack, B's locals are on B's stack. Stacks must not overlap. For embedded systems, typically 256-512 bytes per task stack.

### Inter-Process Communication (IPC)
Tasks need to synchronize and share data safely. Common mechanisms: message queues (task sends data to queue, another reads it), semaphores (counting synchronization primitive), mutexes (mutual exclusion locks), events (signal that something happened).

---

## Architecture

### Task Control Block (TCB)
Data structure holding everything the scheduler needs to know about a task:
- Task ID
- Current stack pointer (saved register state)
- Program counter (where task paused)
- Priority level
- State (running, ready, blocked, suspended)
- Stack base address
- Entry point function pointer
- Any data task needs (semaphores it owns, queues it waits on)

### Ready Queue
List of tasks ready to execute. Ordered by priority or in round-robin order. Scheduler picks task from front of queue.

### Blocked/Waiting Queue
Tasks waiting on resources (mutex, semaphore, message). When resource becomes available, task moves back to ready queue.

### Timer/Interrupt Handler
Fires periodically (every 1ms, 10ms, etc.) to trigger context switch. Calls scheduler to pick next task. Could be hardware timer interrupt or software tick.

---

## Implementation Steps

### Step 1: Define Task Control Block
Create a struct holding task metadata: ID, stack pointer, program counter, priority, state, stack address. You'll create one TCB per task at startup.

### Step 2: Allocate Task Stacks
Reserve memory regions for each task's stack. For 4 tasks with 256 bytes each, allocate 1KB. Initialize each stack with dummy return addresses at the bottom (so if task accidentally returns, it doesn't crash).

### Step 3: Implement Task Creation
Function that takes task entry point (function to run), priority, stack size. Creates TCB, initializes stack, adds to ready queue. Returns task ID.

### Step 4: Implement Scheduler
Decide scheduling policy (round-robin easiest for first version). Maintain ready queue. Scheduler function picks next task from queue. If current task still runnable, put it back in queue. If task blocked, don't queue it.

### Step 5: Implement Context Switch
Low-level assembly routine (ARM, RISC-V, x86 depending on hardware). Save all CPU registers to current task's stack. Update TCB with new stack pointer. Load next task's registers from its stack. Jump to its program counter. This is THE critical piece — everything depends on this.

### Step 6: Implement Timer Interrupt
Set up hardware timer to fire periodically. Interrupt handler calls scheduler, triggers context switch. Everything else follows from this.

### Step 7: Implement Task Yield
Function that voluntarily gives up CPU time. Saves state, calls scheduler, context switches. Allows cooperative multitasking.

### Step 8: Implement Semaphores (Basic IPC)
Counting semaphore: init(count), wait() (decrement, block if zero), signal() (increment, wake waiting task). Used for synchronization between tasks.

### Step 9: Implement Message Queue (Optional but useful)
FIFO queue of messages. Tasks can enqueue and dequeue. Readers block if queue empty, writers block if queue full. Basic producer-consumer pattern.

### Step 10: Implement Idle Task
Task that runs when nothing else is ready. Usually just loops. Prevents scheduler from crashing if all tasks are blocked.

---

## Key Design Decisions

### Stack Direction
Does stack grow up or down in memory? ARM/x86 typically grow down (SP decreases as you push). RISC-V varies. Decide early and be consistent.

### Register Saving
Save all registers or just callee-saved? Saving all is safer but slower. Most RTOS save minimal set and rely on calling conventions.

### Tick Frequency
How often does timer interrupt fire? 1ms is common, 10ms is coarser but uses less power. Trade-off between responsiveness and overhead.

### Number of Priority Levels
8 levels (0-7) is typical. Higher number = higher priority. Task switching only happens if higher-priority task becomes ready.

### Task Lifecycle
States: Created → Ready → Running → Blocked → Ready → Running → Terminated. Track which state each task is in.

---

## Testing Strategy

### Unit Tests
- Create multiple tasks, verify each runs
- Verify task priorities respected (higher priority runs first)
- Verify context switch saves/restores state correctly
- Verify semaphores block and unblock correctly

### Integration Tests
- Producer/consumer pattern with message queue
- Multiple tasks reading same sensor via semaphore
- Task with higher priority interrupts lower priority
- All tasks complete without deadlock

### Stress Tests
- Run for hours, verify no memory corruption
- Rapid task creation/destruction
- Message queue overflow/underflow conditions
- Simultaneous semaphore waits from multiple tasks

---

## What You're Building

A minimal but complete RTOS with:
- 4-8 concurrent tasks
- Priority-based or round-robin scheduling
- Context switching (works on real hardware)
- Basic semaphore for synchronization
- Optional message queue for IPC
- Runs on RISC-V processor or ESP32
- ~500-1000 lines of C++ for scheduler + task management
- ~100-200 lines of assembly for context switch

---

## Why This Is Strong for Resume

- Shows you understand **real-time constraints** (crucial for autonomous, robotics, embedded)
- Demonstrates **low-level systems thinking** (assembly, memory management, interrupt handling)
- Proves you can **write production-grade code** (synchronization, no race conditions, proper error handling)
- **Directly applicable** to job interviews ("How does FreeRTOS work?" → you can explain from first principles)
- **Looks impressive** — most Year 2 students haven't built this

---

## Timeline (4-5 weeks)

**Week 1:** Design TCB, allocate stacks, task creation
**Week 2:** Scheduler implementation, round-robin logic
**Week 3:** Context switch assembly, timer interrupt hookup
**Week 4:** Semaphores, testing, debugging on hardware
**Week 5:** Documentation, GitHub polish, demo video (optional but strong)

---

## Things to Remember

Context switch is hard and error-prone. Test it obsessively. Interrupt safety matters — disable interrupts during scheduler changes. Stack overflow is silent and deadly — give tasks enough stack but not too much. Synchronization primitives must be correct — race conditions are subtle. Document everything — future you will thank you. Test on real hardware, not just simulation.