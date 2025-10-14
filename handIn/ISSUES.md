# Known Issues & Technical Debt

This document tracks the deficiencies identified during the latest code review. File references use workspace-relative paths with line numbers measured after running `make all`.

## Concurrency Correctness

- **Shared iteration counter races** – Every benchmark loop relies on a single unsynchronised `int successCheck` that all threads increment (`framework/src/TAS_BM.c:43`, `framework/src/CLH_BM.c:59`, `framework/src/MCS_BM.c:82`, `framework/src/hemlock_BM.c:65`, `framework/src/array_BM.c:58`, `framework/src/ticket_BM.c:47`, `framework/src/ompCritical_BM.c:16`, `framework/src/ompLock_BM.c:17`). This causes undefined behaviour: threads can overshoot the intended iteration count, and the measured totals become unreliable.
- **Thread-local state missing for queue locks** – The CLH lock allocates a fresh node on every acquisition but never frees it, and it reuses `clh_lock->node` across owners (`framework/src/CLH_BM.c:33-73`). The MCS lock stores the current owner in a single shared pointer `mcs_lock->node`, so releases can wake the wrong thread and leak nodes (`framework/src/MCS_BM.c:37-74`). The Hemlock variant keeps per-thread state in a static TLS node but waits on `pred->grant` transitioning back to `NULL`, which deviates from the algorithm and can stall (`framework/src/hemlock_BM.c:35-56`).

## Atomic Misuse

- **TAS/TATAS type mismatch** – Both locks declare their flag as `atomic_int` yet call `atomic_flag_test_and_set` / `atomic_flag_clear` on it (`framework/src/TAS_BM.c:12-39`, `framework/src/TATAS_BM.c:12-42`). The C11 API requires `atomic_flag`; the current code exhibits undefined behaviour on conforming compilers.
- **Ticket lock spin on non-atomic state** – The ticket lock increments `lock->ticket` atomically but spins on the plain `int *served` without any atomic loads (`framework/src/ticket_BM.c:25-39`). Readers and writers race, making the lock incorrect under optimisation or weak memory.
- **Array lock flag storage** – The array lock stores flags in a `bool *` but drives them through `atomic_load/atomic_store` (`framework/src/array_BM.c:11-49`). Unless the allocation is upgraded to hold `_Atomic bool`, the loads/stores are undefined.
- **Invalid memory-order enum** – The ticket lock passes literal `0` as the memory order to `atomic_fetch_add_explicit` (`framework/src/ticket_BM.c:25`). This compiles by accident but violates the C standard.

## Measurement Integrity

- **Wait time overwritten** – Some locks assign instead of accumulate wait durations (`framework/src/MCS_BM.c:86`, `framework/src/hemlock_BM.c:69`), leading to misleading averages.
- **Fairness and throughput normalisation** – Aggregation divides by the global `times` argument even though the real number of successful acquisitions can differ due to the race on `successCheck` (`framework/src/TAS_BM.c:88-89`, etc.), so per-thread metrics skew once contention starts.

## Resource Management

- **Leaked allocations** – CLH and MCS locks allocate nodes without freeing all of them (`framework/src/CLH_BM.c:33-70`, `framework/src/MCS_BM.c:32-74`), steadily exhausting memory in longer runs.
- **Job script typos** – The SLURM scripts intended for Nebula misspell `mkdir` as `mdkri`, causing job failures unless the directory already exists (`framework/nebulaSlurmHi.sh:19`, `framework/nebulaSlurmLo.sh:19`).

## Build & Tooling

- **Duplicate `make` targets** – The Makefile defines targets literally named `make small-bench` / `make small-plot`, overriding the built-in meaning of `make` (`framework/Makefile:94-121`). GNU Make warns about the collision and the recipes do not persistently activate the virtual environment, so Python dependencies install to the global interpreter.
- **Thread scan baseline gap** – The benchmark harness only tests even thread counts starting at two (`framework/benchmake.py:14-20`); a single-thread baseline is missing, which hampers comparisons.

## Follow-Up

The fixes should prioritise making each lock implementation semantically correct (proper atomics, per-thread state, memory management), then repairing the benchmarking harness to remove the shared counters and to report accurate statistics. Once correctness is restored, revisit the Makefile and SLURM scripts so automated runs become repeatable.
