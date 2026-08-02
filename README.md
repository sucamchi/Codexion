*This project has been created as part of the 42 curriculum by scamlett.*

# Codexion

## Description

Codexion is a concurrency project based on the [dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem). The goal is to coordinate multiple coders (threads) competing for limited dongles (resources) while respecting cooldowns, logging rules, and a selected scheduling policy.

Each coder alternates between compiling, debugging, and refactoring. Compiling requires two dongles at the same time. The simulation ends when either one coder burns out or every coder has completed the required number of compiles.

## Instructions

### Build

```bash
make
```

### Clean

```bash
make clean
make fclean
```

### Run

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

The scheduler must be either `fifo` or `edf`:
- FIFO (first in first out) serves requests in the order they arrive.
- EDF (earliest deadline first) serves the coder with the earliest burnout deadline first.

### Examples:

Below are some example commands to run the simulation with different parameters, testing various scenarios and edge cases:

```bash
./codexion 4 800 200 200 200 3 50 fifo #default case

./codexion 3 300 200 150 150 2 50 edf # EDF case

./codexion 5 1000 250 200 200 3 100 fifo # High contention case

./codexion 1 500 100 100 100 2 50 fifo # Single coder case

./codexion 2 250 200 150 150 2 50 fifo # Forced burnout case

./codexion 4 1000 200 150 150 2 0 fifo # Zero cooldown case

./codexion 6 1500 300 200 200 4 75 edf # High contention with EDF

./codexion 8 2000 200 100 100 1 50 fifo # Large number of coders with minimal work
```
Valgrind can be used to check for memory leaks:

```bash
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./codexion 4 800 200 200 200 3 50 fifo
 ```

 ## Blocking cases handled

- Deadlock prevention by avoiding circular dongle acquisition patterns. Odd-numbered coders take left first, even-numbered coders take right first. This ensures that no two coders are waiting on each other indefinitely.
- Starvation control through FIFO/EDF arbitration on each dongle.
- Precise burnout detection using timestamps and a monitor thread, even if a coder is in debug or refactor.
- Logging is protected by a mutex to prevent messages from different threads from interleaving (mixing).
- Single-coder handling, where the coder uses the only dongle on the table.

## Thread synchronization mechanisms

- `pthread_mutex_t` protects each dongle state, the global log output, and shared simulation flags.
- `pthread_cond_t` is used as a start barrier and to wake waiting coders when a dongle becomes available.
- The dongle wait queue is implemented as a min-heap so the selected scheduler can decide which waiting coder is served next.

## Race-condition prevention

Fields like `sim->stop`, `coder->compiles_done`, and `coder->last_compile_start`
are sometimes read without holding a mutex. This is intentional:

- `sim->stop` is only ever set to `1` — never toggled back — so a slightly
  late read is harmless; the thread will see it on the next check.
- `compiles_done` and `last_compile_start` are only written by the coder
  that owns them, and only read by the monitor (which polls every 1 ms).
- When `stop` is set, the monitor broadcasts on every dongle's condition
  variable, so any thread sleeping on a dongle wakes up and re-checks `stop`
  immediately.

It makes the code cleaner and easier to read, and it avoids unnecessary mutex contention, adding at most 1 ms of delay before a thread notices the stop condition.

## Execution flow

The program runs in this order:

1. `main()` starts the program and calls `parse_args()` to check the command-line values.
2. If the arguments are valid, `main()` calls `init_sim()` to create the coders, dongles, mutexes, and condition variable.
3. `main()` then calls `launch_threads()` to start every coder thread and the monitor thread.
4. Each coder thread runs `coder_lifecycle()`, which first waits in `coder_wait_start()` until every thread is ready.
5. When the simulation starts, each coder repeats `coder_do_cycle()` until it has done enough compiles or the simulation stops.
6. Inside each cycle, the coder takes dongles with `coder_take_dongles()`, which uses `coder_pick_dongles()` and `dongle_take()` to avoid deadlocks.
7. After both dongles are taken, the coder compiles, releases the dongles with `dongle_release()`, and then continues with the debug and refactor steps.
8. At the same time, the monitor thread runs `monitor_run()`, which waits for the start signal and keeps calling `monitor_check_state()` to look for burnout or for everyone finishing.
9. If a coder burns out, `monitor_run()` sets `sim->stop` to `1`, logs the burnout, and wakes all coders so they can exit their cycles.
10. When all threads finish, `main()` calls `cleanup_sim()` to join the threads and free every resource, avoiding memory leaks.


## How it works

### Overall structure

The program uses 7 source files, each with a specific job:

| File | Job |
|------|-----|
| `main.c` | Parses command-line arguments, starts the simulation |
| `simulation.c` | Creates and destroys all coders, dongles, and mutexes |
| `coder.c` | Each coder's life: wait → grab dongles → compile → release → debug → refactor → repeat |
| `dongle.c` | A shared USB dongle with a mutex, a waiting queue, and a cooldown timer |
| `heap.c` | A min-heap (priority queue) used as the dongle's waiting queue |
| `monitor.c` | A monitor thread that checks if anyone burned out |
| `utils.c` | Time helpers, logging, sleep, heap peek/clear |


### The startup barrier (all threads wait for "go")

Every coder thread and the monitor thread call a `wait_start()` function.
They all lock the same mutex, increment a `ready_count`, and then wait on
a condition variable. The last thread to arrive broadcasts the condition,
which wakes up `main`. `main` then sets a `started` flag and broadcasts
again — this wakes up all coders and the monitor simultaneously.


### How a coder lives and dies

A coder repeats the cycle of compiling, debugging, and refactoring until either they have compiled enough or the simulation stops when another coder burns out. The monitor thread checks every coder's `last_compile_start` timestamp to see if they have exceeded `time_to_burnout`. If so, the monitor sets the shared `sim->stop` flag and wakes all coders so they can exit their cycles.

### How a coder grabs two dongles (deadlock prevention)

A coder needs the dongle on their left AND the dongle on their right.
To avoid deadlock (everyone holding one and waiting forever):

- Even-numbered coders grab **right first, then left**
- Odd-numbered coders grab **left first, then right**

This breaks the circular dependency — coders 1 and 2 can't both be
waiting on each other's dongle because they reach for different ones first.

If there's only one coder (n = 1), both `left` and `right` point to the
same dongle — they just take it once.


### How the dongle waiting queue works

Each dongle has a **min-heap** (priority queue) as its waiting list.
Coders join the queue with a priority value:

- **FIFO**: priority = the current timestamp (earliest wins)
- **EDF**: priority = `last_compile_start + time_to_burnout` (earlier deadline wins)

This heap always keeps the smallest priority at the top.


### How the cooldown works

When a coder releases a dongle, the dongle becomes unavailable for
`dongle_cooldown` milliseconds. The dongle's `cooldown_end` timestamp is set to the current time plus the cooldown. Any coder trying to take the dongle while it is cooling down will be blocked.
Any coder that is waiting on a dongle will be automatically woken up when the cooldown ends, and they will re-check if they can take the dongle.

### How the monitor detects burnout

The monitor thread wakes up every 1 ms and checks each coder's `last_compile_start` timestamp. If the current time minus that timestamp exceeds `time_to_burnout`, the monitor sets the shared `sim->stop` flag to `1`, logs the burnout, and wakes all coders so they can exit their cycles.


### How the min-heap works (the priority queue)

A min-heap is a binary tree stored in an array where the **smallest**
element is always at position `[0]`. Two internal helpers maintain
this property:

- **sift_up**: when adding a new element at the end, bubble it up by
  swapping with its parent until the heap property is restored.
- **sift_down**: after removing the root (position 0), take the last
  element, put it at the top, and bubble it down by swapping with the
  smaller child.


### How time is measured

Instead of using absolute Unix timestamps (which might overflow `int`),
we capture the current time **once** at startup in `sim->base_time`.
Every `get_time_ms()` call calculates the elapsed milliseconds since that base time. This keeps all timestamps small and relative to the simulation start.


### How logging stays clean

`log_msg()` locks a dedicated mutex before printing.
This guarantees that two threads can't print at the same time and
mix their output.


## Resources

- 42 Codexion subject.
- 42 Philosophers subject. (previous version of this project)
- POSIX threads documentation for `pthread_create`, `pthread_mutex_*`, and `pthread_cond_*`.
- `gettimeofday` and `usleep` documentation 
- [Dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)
- [Concurrency](https://en.wikipedia.org/wiki/Concurrency_(computer_science))
- [Pthreads](https://en.wikipedia.org/wiki/Pthreads)
- https://medium.com/@denaelgammal/dining-philosophers-problem-42-project-guide-mandatory-part-a20fb8dc530e
- https://dev.to/yel-bakk/codexion-4fk8
- https://42-fran-byte-f94097.gitlab.io/docs/philosophers/philosophers-approach-es/#/
- Book: Pthreads Programming: A POSIX Standard for Better Multiprocessing by B Nichols, D. Buttlar, J. Farrell

### AI usage

AI was used to help format the README.md, to test edge cases, and to provide guidance on how to implement the project.
