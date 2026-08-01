*This project has been created as part of the 42 curriculum by scamlett.*

# Codexion

## Description

Codexion is a concurrency project based on the [dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem). The goal is to coordinate multiple coder threads competing for shared dongles while respecting cooldowns, timing constraints, logging rules, and a selected scheduling policy.

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

```bash
./codexion 4 800 200 200 200 3 50 fifo
```


EDF scheduling with cooldown and a short burnout window:

```bash
./codexion 3 600 200 150 150 2 50 edf
```

High contention with more coders and a longer cooldown:

```bash
./codexion 5 1000 250 200 200 3 100 fifo
```


## Blocking cases handled

- Deadlock prevention by avoiding circular resource acquisition patterns.
- Starvation control through FIFO/EDF arbitration on each dongle.
- Precise burnout detection using timestamps and a monitor thread.
- Log serialization so state messages never interleave.
- Single-coder handling, where the coder uses the only dongle on the table.

## Thread synchronization mechanisms

- `pthread_mutex_t` protects each dongle state, the global log output, and shared simulation flags.
- `pthread_cond_t` is used as a start barrier and to wake waiting coders when a dongle becomes available.
- The dongle wait queue is implemented as a heap so the selected scheduler can decide which waiting coder is served next.

## A note on the shared stop/progress flags

`sim->stop`, `coder->compiles_done`, and `coder->last_compile_start` are read
in places that don't hold a mutex — for example, `coder_lifecycle`'s main
loop checks `sim->stop` directly, and `dongle_take` checks it while holding
only the dongle's own mutex rather than `sim_mutex`.

This is a deliberate trade-off, not an oversight:

- These fields are plain `int`s, and on every architecture this project
  targets, an aligned `int` read or write is a single atomic bus operation —
  there's no possibility of a torn read where one thread sees half an old
  value and half a new one.
- The actual *correctness* of wakeups doesn't depend on these reads at all.
  A coder blocked in `dongle_take` is only ever woken by
  `pthread_cond_broadcast` (on release, on a new arrival, or from
  `monitor_stop_all` when the monitor sets `stop`). The unguarded read of
  `stop` is just the condition the thread re-checks *after* waking up — the
  condition variable and its associated mutex are what guarantee the thread
  doesn't miss a wakeup or race on the dongle's own state.
- Because every coder and the monitor also re-check these flags at least
  once every loop iteration (the monitor polls every 1 ms), the worst case
  is a thread noticing `stop` a few microseconds later than another thread
  set it — never a stale value it never notices at all.

The formally "correct" alternative would be to wrap every read of `stop`,
`compiles_done`, and `last_compile_start` in `sim_mutex`. We chose not to,
because it would mean the monitor and every dongle wait loop taking a third,
unrelated mutex on every single poll, adding lock contention that works
against the sub-10ms burnout precision requirement, in exchange for a
guarantee that doesn't change any observable behavior on real hardware.
If this project were extended beyond x86/ARM word-atomic reads, or built
with a stricter memory model in mind, these flags would need to move to
`_Atomic int` or be brought under `sim_mutex`.

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
9. If a coder burns out, the monitor sets the stop flag with `set_stop()` and wakes any waiting coders with `monitor_stop_all()`.
10. When all threads finish, `main()` calls `cleanup_sim()` to join the threads and free every resource.

![Execution flow diagram](https://media2.dev.to/dynamic/image/width=800%2Cheight=%2Cfit=scale-down%2Cgravity=auto%2Cformat=auto/https%3A%2F%2Fdev-to-uploads.s3.amazonaws.com%2Fuploads%2Farticles%2Fz9isrrnm8su4bn265b9v.png)

## How it works

### Overall structure

The program uses 8 source files, each with a specific job:

| File | Job |
|------|-----|
| `main.c` | Parses command-line arguments, starts the simulation |
| `simulation.c` | Creates and destroys all coders, dongles, and mutexes |
| `coder.c` | Each coder's life: wait → grab dongles → compile → release → debug → refactor → repeat |
| `dongle.c` | A shared USB dongle with a mutex, a waiting queue, and a cooldown timer |
| `heap.c` | A min-heap (priority queue) used as the dongle's waiting queue |
| `monitor.c` | A watchdog thread that checks if anyone burned out |
| `log.c` | Thread-safe printing so messages never get tangled |
| `utils.c` | Time helper (elapsed ms since start), sleep, heap peek/clear |

### The startup barrier (all threads wait for "go")

Every coder thread and the monitor thread call a `wait_start()` function.
They all lock the same mutex, increment a `ready_count`, and then wait on
a condition variable. The last thread to arrive broadcasts the condition,
which wakes up `main`. `main` then sets a `started` flag and broadcasts
again — this wakes up **all** coders and the monitor simultaneously.

```
each coder + monitor:
    lock(mutex)
    ready_count++
    if i'm the last one → broadcast(start_cond)
    while not started → wait(start_cond)
    unlock(mutex)

main:
    create all threads
    lock(mutex)
    while ready_count < total_threads → wait(start_cond)
    started = true
    broadcast(start_cond)
    unlock(mutex)
```

### How a coder lives and dies

A coder repeats this cycle until they've compiled enough times, or the
simulation stops (someone burned out):

```
coder_lifecycle(coder):
    wait_start()                    // join the barrier
    last_compile_start = 0          // timer starts at 0

    while simulation is running and i haven't compiled enough:
        if not take_both_dongles():
            break                   // simulation stopped

        last_compile_start = now()  // reset the burnout clock
        log "is compiling"
        sleep(time_to_compile)

        release(left_dongle)
        release(right_dongle)
        compiles_done++

        if i've compiled enough → continue

        log "is debugging"
        sleep(time_to_debug)
        log "is refactoring"
        sleep(time_to_refactor)
```

### How a coder grabs two dongles (deadlock prevention)

A coder needs the dongle on their left AND the dongle on their right.
To avoid deadlock (everyone holding one and waiting forever), we use
**asymmetric ordering**:

- Even-numbered coders grab **right first, then left**
- Odd-numbered coders grab **left first, then right**

This breaks the circular dependency — coders 1 and 2 can't both be
waiting on each other's dongle because they reach for different ones first.

If there's only one coder (n = 1), both `left` and `right` point to the
same dongle — they just take it once.

```
take_both_dongles(coder):
    if scheduler is EDF:
        priority = last_compile_start + time_to_burnout   // earlier deadline wins
    else (FIFO):
        priority = now()                                   // earlier arrival wins

    if coder is even → first = right, second = left
    if coder is odd  → first = left,  second = right

    if first == second:                     // single-coder case
        take(first)                         // take the only dongle
        log "has taken a dongle" twice
        return success

    take(first)                             // block until first dongle is mine
    log "has taken a dongle"
    take(second)                            // block until second dongle is mine
    log "has taken a dongle"
    return success
```

### How a dongle decides who gets it (the waiting queue)

Each dongle has a **min-heap** (priority queue) as its waiting list.
Coders join the queue with a priority value:

- **FIFO**: priority = the current timestamp (earliest wins)
- **EDF**: priority = `last_compile_start + time_to_burnout` (earlier deadline wins)

The heap always keeps the smallest priority at the top.

```
take(dongle, my_id, my_priority):
    lock(dongle.mutex)

    push myself into the heap with my_priority
    if i'm at the front → broadcast to wake other waiters

    while simulation is running:
        peek at who's at the front
        if it's me AND the dongle is free AND cooldown is over:
            break                          // i got it!

        if i'm at the front but cooldown isn't over:
            timedwait until cooldown ends  // sleep with a timer
        else:
            cond_wait                      // sleep until someone wakes me

    if simulation is still running:
        pop myself from the heap           // leave the queue
        dongle.taken_by = my_id            // mark as taken by me

    unlock(dongle.mutex)
    return whether simulation is still running
```

### How the cooldown works

When a coder releases a dongle, the dongle becomes unavailable for
`dongle_cooldown` milliseconds:

```
release(dongle):
    lock(dongle.mutex)
    dongle.taken_by = 0
    dongle.cooldown_until = now() + dongle_cooldown
    broadcast                            // wake ALL waiters
    unlock(dongle.mutex)
```

Any waiter at the front of the queue who sees the dongle is free but
still cooling down will sleep with `pthread_cond_timedwait` — they'll
wake up automatically when the cooldown expires.

### How the monitor detects burnout

A separate thread (the monitor) wakes up every 1 millisecond and
checks every coder:

```
monitor_run():
    wait_start()                          // join the barrier

    while simulation is running:
        result = check_state()

        if result == -1:                  // everyone done
            stop simulation, break

        if result > 0:                    // someone burned out
            stop simulation
            log "X burned out"
            wake all coders (so they exit)
            return

        sleep(1 ms)                       // check again soon

check_state():
    if every coder has compiled enough → return -1   // all done

    for each coder:
        if they haven't finished AND
           (now - last_compile_start) > time_to_burnout:
            return that coder's ID                  // burnout!
    return 0                                         // keep going
```

### How the min-heap works (the priority queue)

A min-heap is a binary tree stored in an array where the **smallest**
element is always at position `[0]`. Two internal helpers maintain
this property:

- **sift_up**: when adding a new element at the end, bubble it up by
  swapping with its parent until the heap property is restored.
- **sift_down**: after removing the root (position 0), take the last
  element, put it at the top, and bubble it down by swapping with the
  smaller child.

```
push(heap, coder_id, priority):
    add {coder_id, priority} at the end of the array
    sift_up from that position
    size++

pop(heap):
    move the last element to position 0
    sift_down from position 0
    size--

peek(heap):       // look without removing
    return the element at position 0
```

### How time is measured

Instead of using absolute Unix timestamps (which would overflow `int`),
we capture the current time **once** at startup in `sim->base_time`.
Every `get_time_ms()` call subtracts that baseline — so all timestamps
start at 0 and grow by the actual elapsed milliseconds. Since the
simulation runs for seconds, not years, values stay tiny:

```
get_time_ms(sim):
    now = current time
    sec  = now.seconds - base_time.seconds
    usec = now.microseconds - base_time.microseconds
    if usec went negative → borrow 1 second
    return sec * 1000 + usec / 1000
```

### How logging stays clean

All log messages go through `log_msg()`, which locks a dedicated mutex
before printing. This guarantees that two threads can't print at the
same time and mix their output:

```
log_msg(sim, message, coder_id):
    lock(log_mutex)
    print elapsed_ms, coder_id, message
    unlock(log_mutex)
```

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
- Books: C Programming: A Modern Approach (2nd Edition) by K. N. King and Pthreads Programming: A POSIX Standard for Better Multiprocessing by B Nichols, D. Buttlar, J. Farrell

### AI usage

AI was used to help draft the README structure, as a tutor and to help refactor functions to adjust to the project requirements and norminette.
