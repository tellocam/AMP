# Advanced Multiprocessor Lock Benchmarks

This repository contains the final submission for the Advanced Multiprocessor (AMP) course project by Camilo Tello Fachin, Manuela Maria Raidl, and Iris Grze. It provides a collection of lock implementations written in C with OpenMP, Python tooling to benchmark them across different machines, and scripts to visualise the collected data.

## Project Layout

```
handIn/
├── NebulaData/         # Raw benchmark results generated on Nebula (text files)
├── framework/          # Source code, build system, benchmarking scripts
│   ├── include/        # Shared C headers
│   ├── src/            # Lock implementations and benchmarking helpers
│   ├── plots/          # Generated figures (populated by plotting scripts)
│   ├── data/           # Local benchmark output (created at runtime)
│   ├── Makefile        # Build and automation targets
│   ├── bench*.py       # Benchmark entry-points
│   └── utils.py        # Plotting helpers and ctypes bridge definitions
└── plots/              # Submitted plots ready for the report
```

Refer to `ISSUES.md` for the catalogue of outstanding technical problems and suggested fixes.

## Prerequisites

- GCC with OpenMP support (`gcc -fopenmp`)
- GNU Make
- Python 3.8+ with `ctypes`, `numpy`, `pandas`, and `matplotlib`
- Access to an OpenMP-capable CPU (benchmarks query available hardware using `nproc`)
- Optional: SLURM environment with Nebula’s `q_student` partition for the provided job scripts

## Building the Shared Library

All locks are compiled into a single shared object (`build/sharedLibrary.so`). From `handIn/framework/` run:

```bash
mkdir -p build
make all
```

The Makefile compiles every `src/*.c` file with `-fopenmp -fPIC` and links them into the shared library that the Python benchmarks load through `ctypes`.

## Running Benchmarks Locally

Create the Python virtual environment and capture a short benchmark/plot cycle:

```bash
make small-bench
make small-plot
```

These targets install Python dependencies inside `myenv/`, execute `benchmake.py` to produce fresh `.txt` datasets in `framework/data/`, and render `plots/small-plot.pdf`. Adjust `total_acqs` or `bench_iters` inside `benchmake.py` if you need different workloads.

To benchmark or stress a specific lock manually:

```bash
python3 benchnebula.py <threads> <lock_acquisitions> <iterations> <sleep_cycles> <lock-name>
python3 benchContention.py <threads> <lock_acquisitions> <iterations> <sleep_cycles> <lock-name>
```

- `benchnebula.py` sweeps the number of threads.
- `benchContention.py` sweeps the number of post-critical-section sleep cycles.

Both scripts emit tab-separated files into `framework/data/` with column names that match the plotting utilities.

## Plotting Existing Data

If the `.txt` files are already present (for example from Nebula runs), regenerate figures without rerunning the benchmarks:

```bash
python3 plotmake.py
```

Generated plots are placed in `framework/plots/`, while the submission-ready figures reside in the top-level `plots/` directory.

## Working on Nebula

The SLURM job scripts in `framework/` illustrate how the benchmarks were executed on TU Wien’s Nebula cluster. Before submitting them:

1. Update the resource parameters (`thr`, `acq`, `iter`, `sc`) to the desired workload.
2. Ensure the `build/`, `data/`, and `myenv/` directories are created in the job script or pre-existing in your workspace.
3. Submit with `sbatch nebulaSlurmHi.sh` (or its `Lo`/`Cont` variants) from within `framework/`.

Submitted results appear under `NebulaData/` so they can be versioned alongside the report.

## Extending the Project

- Add new lock implementations by placing additional `*_BM.c` files in `src/` and exposing them through the shared library.
- Extend the Python benchmarking harness by registering new `restype` mappings in `benchmake.py` and wiring them into the dataframe builder.
- Tailor the plotting script (`plotmake.py`) for alternative visualisations, e.g., log-scale or per-metric comparisons.

For known limitations and the roadmap of fixes, consult `ISSUES.md`.
