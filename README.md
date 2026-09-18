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

### Starvation prevention

The dongle waiting queues use a heap. The selected coder is determined by the configured scheduler:

- FIFO prioritizes earlier arrivals.
- EDF prioritizes the coder with the earliest burnout deadline.

The `last_dgl_granted` value is used as a tie-breaker when EDF deadlines are equal, helping avoid repeatedly selecting the same coder.

### Cooldown handling

After a dongle is released, it cannot immediately be reused. The `released_time` value records when it became available, and the cooldown duration is checked before another coder can acquire it.

This prevents a dongle from being reused before its configured cooldown period has elapsed.

### Burnout detection

The monitor thread periodically reads each coder’s `last_compile` value. If the time since the coder’s last compilation exceeds `time_to_burnout`, the simulation stops and the coder is reported as burned out.

### Completion handling

The monitor checks whether every coder has completed the required number of compilations. When all coders are finished, the simulation stops and waiting threads are notified.

### Log serialization

All log messages are protected by `mutex_sim`. This prevents multiple coder threads from writing to standard output simultaneously and keeps each event message intact.

## Thread synchronization mechanisms

### `pthread_mutex_t`

Mutexes protect shared state accessed by multiple threads.

The project uses:

- A mutex for each dongle.
- A mutex for each coder.
- A simulation mutex for shared simulation state and logging.

Examples:

- Dongle availability, cooldown timestamps, and waiting heaps are accessed while holding the dongle mutex.
- `last_compile`, `coder_compiles_num`, and other coder state are protected by the coder mutex.
- `sim_stopped` and output operations are protected by the simulation mutex.

This prevents data races such as one thread updating `last_compile` while the monitor reads it.

### `pthread_cond_t`

Each dongle contains a condition variable intended to notify waiting coders when the dongle state changes.

A dongle release broadcasts a notification after updating its availability and release time. Waiting threads can then recheck whether they are allowed to continue.

Condition variables must always be used together with the mutex protecting the related condition state.

### Custom event implementation

The project does not use a separate custom event abstraction. Thread communication is implemented using POSIX mutexes, condition variables, and simulation-state checks.

The monitor communicates a global stop condition by updating `sim_stopped` under `mutex_sim` and waking waiting threads. Coder threads periodically check this state and stop their work when the simulation has ended.

### Monitor and coder communication

The monitor thread:

1. Locks a coder’s mutex.
2. Reads `last_compile`.
3. Unlocks the coder’s mutex.
4. Detects burnout or completion.
5. Updates `sim_stopped` under the simulation mutex.
6. Wakes waiting threads.

Coder threads check `sim_stopped` before and during their activities. This allows the monitor to stop the simulation without directly modifying coder-thread control flow.

## Project structure

- `main.c`: initializes the simulation, creates threads, joins threads, and cleans up resources.
- `coder_journey.c`: implements the coder lifecycle.
- `get_dongle.c`: handles dongle acquisition.
- `let_dongle.c`: releases dongles.
- `monitor_journey.c`: monitors burnout and completion.
- `schedulers.c`: implements FIFO and EDF priority logic.
- `heap_op.c`: manages the dongle waiting heaps.
- `inits.c`: initializes coders and dongles.
- `parser.c`: validates command-line arguments.
- `codexion.h`: contains shared structures and function declarations.
- `Makefile`: provides build and cleanup commands.

## Resources

- Understanding the dining philosophers problem:
  https://en.wikipedia.org/wiki/Dining_philosophers_problem

- Thread versus Process:
  https://www.youtube.com/watch?v=1myWEH8IGt4
  https://www.youtube.com/watch?v=PgDaJEjlBuI
  https://www.youtube.com/watch?v=4rLW7zg21gI
  
- POSIX Threads documentation:
  https://en.wikipedia.org/wiki/Pthreads 
  https://www.youtube.com/watch?v=ldJ8WGZVXZk

- `pthread_mutex_lock` documentation:  
  https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3.html
  https://www.youtube.com/watch?v=raLCgPK-Igc
  
- Heap documentation:
  https://www.youtube.com/watch?v=Dvq-YKeuO9Y&t=762s
  https://www.youtube.com/watch?v=9q4AQFiSOLU
  https://www.youtube.com/watch?v=XycnarZEBvQ&t=11s

- Valgrind Helgrind documentation:  
  https://valgrind.org/docs/manual/hg-manual.html

- Coffman deadlock conditions:
  https://www.youtube.com/watch?v=ElXO5cGBDEs
  https://en.wikipedia.org/wiki/Deadlock

### AI usage

AI assistance was used as a programming support tool for:

- Interpreting Helgrind data-race and synchronization warnings.
- Understanding some theorical concepts such as binary trees, mutexes, etc.
- Preparing and structuring this documentation.
