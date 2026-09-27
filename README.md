*This project has been created as part of the 42 curriculum by limelo-c.*

# Codexion

## Description

Codexion is a concurrent simulation written in C using POSIX threads. Several coder threads compete for shared dongles in order to compile, debug, and refactor code.

Each coder must:

1. Acquire two dongles.
2. Compile for the configured duration.
3. Release both dongles.
4. Debug.
5. Refactor.
6. Repeat until the required number of compilations is reached or a coder burns out.

The project demonstrates thread creation, mutex synchronization, condition variables, scheduling algorithms, resource management, and race-condition prevention.

Two scheduling policies are supported:

- `fifo`: first-in, first-out scheduling.
- `edf`: earliest-deadline-first scheduling based on each coder’s burnout deadline.

## Instructions

### Requirements

- A C compiler such as `cc` or `gcc`
- POSIX threads
- GNU Make
- Linux or WSL

### Compilation

From the repository root:

```bash
make
```

To perform a clean rebuild:

```bash
make re
```

To remove object files:

```bash
make clean
```

To remove all generated build files:

```bash
make fclean
```

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile \
time_to_debug time_to_refactor number_of_compiles_required \
dongle_cooldown scheduler
```

Example:

```bash
./codexion 5 3000 200 200 200 10 800 edf
```

The scheduler must be either:

```text
fifo
```

or:

```text
edf
```

The program prints timestamped events such as:

```text
125 2 has taken a dongle
125 2 is compiling
326 2 is debugging
527 2 is refactoring
```

## Blocking cases handled

### Deadlock prevention

Coders request their dongles in a consistent order. The lower-numbered dongle is acquired before the higher-numbered dongle. This ordering prevents a circular wait between coders.

Without a fixed ordering, the following Coffman conditions could produce deadlock:

- Mutual exclusion: each dongle can be held by only one coder.
- Hold and wait: a coder holds one dongle while requesting another.
- No preemption: dongles are released voluntarily.
- Circular wait: coders wait in a cycle for each other’s dongles.

The fixed dongle order prevents the circular-wait condition.

The same principle is applied when the scheduler compares two coders' state (arrival time and deadline) under lock: their mutexes are always locked in ascending `coder_id` order, regardless of which coder is being compared to which. This prevents a symmetrical deadlock between two threads simultaneously comparing the same pair of coders in opposite order.

### Starvation prevention

The dongle waiting queues use a heap. The selected coder is determined by the configured scheduler:

- FIFO prioritizes earlier arrivals, so priority only improves the longer a coder waits.
- EDF prioritizes the coder with the earliest burnout deadline, which likewise only becomes more urgent with time.

In both policies, `coder_id` is used as the final tie-breaker whenever arrival times or deadlines are equal, so no comparison is ever ambiguous and no single coder can be repeatedly favored over another by chance.

Coders that are next in priority order but whose dongles are not yet both available and past cooldown are temporarily skipped rather than discarded: they are pushed back onto the waiting heap so they remain in contention on the next scheduling round.

### Cooldown handling

After a dongle is released, it cannot immediately be reused. The `released_time` value records when it became available, and the cooldown duration is checked before another coder can acquire it.

This prevents a dongle from being reused before its configured cooldown period has elapsed.

### Burnout detection

The monitor thread periodically reads each coder’s `last_compile` value. If the time since the coder’s last compilation exceeds `time_to_burnout`, the simulation stops and the coder is reported as burned out.

### Completion handling

The monitor checks whether every coder has completed the required number of compilations. When all coders are finished, `sim_stopped` is set and `wakeup_thread` broadcasts on each dongle's condition variable (see the `pthread_cond_t` note on why this broadcast currently has no waiting listener); coder threads detect the stop themselves via their polling loops.

### Log serialization

All log messages are protected by `mutex_sim`. This prevents multiple coder threads from writing to standard output simultaneously and keeps each event message intact.

## Thread synchronization mechanisms

### `pthread_mutex_t`

Mutexes protect shared state accessed by multiple threads.

The project uses:

- A mutex for each dongle.
- A mutex for each coder.
- A simulation mutex (`mutex_sim`) for shared simulation state and logging.
- A scheduling mutex (`mutex_sched`) for the shared waiting heap.

Examples:

- Dongle availability is checked under the dongle's own mutex in `both_dongles_permission`, and set under the dongle mutexes (plus `mutex_sim`) when a coder is granted both dongles in `got_dongles`. Releasing a dongle updates its availability and `released_time` under `mutex_sched` in `let_dongle`.
- `last_compile`, `coder_compiles_num`, `arrival`, and other per-coder state are protected by that coder's own mutex. The scheduler also locks two coders' mutexes together (in a fixed `coder_id` order) when comparing their state in `priority_coder`.
- `sim_stopped` and `printf` output are protected by `mutex_sim`.
- The waiting heap (`waiting_heap`) is protected by `mutex_sched` everywhere it's pushed to or popped from.

This prevents data races such as one thread updating `last_compile` while the monitor reads it.

### `pthread_cond_t`

Each dongle contains a condition variable, and it is broadcast in two places: `let_dongle` (after a dongle is released) and `wakeup_thread` (when the monitor stops the simulation).

In the current implementation, no thread ever calls `pthread_cond_wait` on these condition variables — there is no blocking consumer. The actual waiting behavior is done by short polling loops instead: `get_both_dongles` retries roughly every 100 microseconds via `usleep`, and `simulation_stopper_helper` does the same while a coder is compiling, debugging, or refactoring, checking `sim_is_stopped()` on each pass. The condition variables are declared, initialized, destroyed, and broadcast on correctly, but as written they do not currently drive any thread's wake-up — that role is filled entirely by the polling loops re-checking shared state under the relevant mutex.

### Custom event implementation

The project does not use a separate custom event abstraction. Thread communication is implemented using POSIX mutexes and simulation-state checks; condition variables are declared and broadcast on but, since no thread calls `pthread_cond_wait`, they are not what actually propagates the stop signal (see `pthread_cond_t` above).

The monitor communicates a global stop condition by updating `sim_stopped` under `mutex_sim` and calling `wakeup_thread` (a broadcast on each dongle's condition variable, currently without effect for the reason above). Coder threads detect the stop by periodically re-reading `sim_stopped` through `sim_is_stopped()` / `sim_is_stopped(codex)` inside their own polling loops, and stop their work once it is set.

### Monitor and coder communication

The monitor thread:

1. Locks a coder’s mutex.
2. Reads `last_compile`.
3. Unlocks the coder’s mutex.
4. Detects burnout or completion.
5. Updates `sim_stopped` under the simulation mutex.
6. Broadcasts on each dongle's condition variable via `wakeup_thread` (see the note under `pthread_cond_t` about why this currently has no waiting listener).

Coder threads check `sim_stopped` before and during their activities, via their own polling loops rather than by waking from a blocked `pthread_cond_wait`. This allows the monitor to stop the simulation without directly modifying coder-thread control flow.

## Project structure

- `main.c`: parses arguments, initializes the simulation, creates the monitor and coder threads, joins them, and triggers cleanup.
- `parser.c`: validates argument count/format and dispatches parsed values into the simulation struct; also validates and sets the scheduler (`fifo`/`edf`).
- `numeric_parser.c`: numeric string validation and overflow checking used by `parser.c` for each numeric argument, plus a variant for the dongle cooldown argument.
- `inits.c`: provides the millisecond timestamp helper and initializes the dongle array/waiting heap and coder array.
- `coder_journey.c`: implements a coder thread's lifecycle loop — attempting to compile (which includes acquiring/releasing dongles), debug, and refactor, while checking for a simulation stop at each stage.
- `get_dongle.c`: coordinates a coder's attempt to acquire both of its dongles, working with the scheduler and waiting heap to determine which coder is granted access.
- `dongles_utils.c`: helper routines used during dongle acquisition — logging a dongle pickup, checking whether the simulation has stopped, checking whether both of a coder's dongles are available and past cooldown, and selecting the next ready coder from the waiting heap.
- `let_dongle.c`: releases a coder's dongles and records their release time for the cooldown check.
- `schedulers.c`: implements the FIFO/EDF priority comparison between two coders and the heap's sift-up/sift-down operations.
- `heap_op.c`: implements the waiting heap's core primitives (creation, push, pop, peek, swap).
- `monitor_journey.c`: the monitor thread's loop — detects coder burnout and overall completion, and stops the simulation when either occurs.
- `cleaners.c`: holds the process-wide `codex_return()` singleton accessor, destroys mutexes/condition variables and frees allocated memory during cleanup, and joins the monitor thread before cleanup runs.
- `codexion.h`: shared structs, enums, and function prototypes used across all source files.
- `Makefile`: build and cleanup rules.

## Resources

### Concepts

- [Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem) - Wikipedia
- [Deadlock / Coffman Conditions](https://en.wikipedia.org/wiki/Deadlock) - Wikipedia
- [POSIX Threads (Pthreads)](https://en.wikipedia.org/wiki/Pthreads) - Wikipedia

### Video explanations (Youtube)

- [Thread vs Process](https://www.youtube.com/watch?v=1myWEH8IGt4)
- [Thread vs Process (alt. explanation)](https://www.youtube.com/watch?v=PgDaJEjlBuI)
- [Thread vs Process (alt. explanation)](https://www.youtube.com/watch?v=4rLW7zg21gI)
- [POSIX Threads walkthrough](https://www.youtube.com/watch?v=ldJ8WGZVXZk)
- [pthread_mutex_lock explained](https://www.youtube.com/watch?v=raLCgPK-Igc)
- [Heaps explained](https://www.youtube.com/watch?v=Dvq-YKeuO9Y&t=762s)
- [Heaps explained (alt. explanation)](https://www.youtube.com/watch?v=9q4AQFiSOLU)
- [Heaps explained (alt. explanation)](https://www.youtube.com/watch?v=XycnarZEBvQ&t=11s)
- [Coffman deadlock conditions](https://www.youtube.com/watch?v=ElXO5cGBDEs)

### Reference documentation

- [pthread_mutex_lock man page](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html)
- [Valgrind Helgrind manual](https://valgrind.org/docs/manual/hg-manual.html)

### AI usage

AI assistance was used as a programming support tool for:

- Interpreting Helgrind data-race and synchronization warnings.
- Understanding some theorical concepts such as binary trees, mutexes, etc.
- Preparing and structuring this documentation.