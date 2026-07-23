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
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
	time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

Example:

```bash
./codexion 4 800 200 200 200 3 50 fifo
```

The scheduler must be either `fifo` or `edf`:
- FIFO serves requests in the order they arrive.
- EDF serves the coder with the earliest burnout deadline first.

## Features

- One thread per coder.
- One shared dongle between each pair of coders.
- Dongle cooldown after each release.
- FIFO or EDF arbitration with a heap-based wait queue.
- Serialized logging to avoid mixed output.
- Separate monitor thread for burnout detection.
- Clean shutdown with resource cleanup.

## Blocking cases handled

- Deadlock prevention by avoiding circular resource acquisition patterns.
- Starvation control through FIFO/EDF arbitration on each dongle.
- Cooldown handling so a released dongle cannot be reused immediately.
- Precise burnout detection using timestamps and a monitor thread.
- Log serialization so state messages never interleave.
- Single-coder handling, where the coder uses the only dongle on the table.

## Thread synchronization mechanisms

- `pthread_mutex_t` protects each dongle state, the global log output, and shared simulation flags.
- `pthread_cond_t` is used as a start barrier and to wake waiting coders when a dongle becomes available.
- The monitor and coder threads communicate through shared state guarded by mutexes, which prevents races on `stop`, `ready_count`, and `last_compile_start`.
- The dongle wait queue is implemented as a heap so the selected scheduler can decide which waiting coder is served next.

## Resources

- 42 Codexion subject.
- 42 Philosophers subject. (previous version of this project)
- POSIX threads documentation for `pthread_create`, `pthread_mutex_*`, and `pthread_cond_*`.
- `gettimeofday(2)` documentation for millisecond timestamps.
- [Dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem)

### AI usage

AI was used to help draft the README structure, summarize the project behavior, and organize the required concurrency and synchronization sections.
