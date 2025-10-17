# Repository Restructuring Roadmap

Transform the AMP coursework submission into a production-ready benchmarking suite. The roadmap is organised into phased milestones so we can evolve the codebase without breaking existing users.

## Guiding Principles
- Keep the current `handIn/` layout buildable until a compatibility layer is formally retired.
- Land changes in feature branches guarded by smoketests/benchmarks so migrations never regress behaviour.
- Separate concerns early: correctness fixes first, structural moves second, packaging/documentation last.

## Phase 0 – Baseline & Triage
**Objectives**
- Freeze the current state and capture reproducible reference points.
- Document implicit assumptions to surface migration risks.

**Key Tasks**
- Snapshot the repository (`baseline/<date>` branch + tag) and export `make all`, `make small-bench`, `make small-plot` outputs under `artifacts/`.
- Inventory directories and scripts inside and outside `handIn/` (locks, Python tooling, Nebula, plots, latex) noting ownership and consumers.
- Record environment requirements (OpenMP flags, Python version, virtualenv name, data paths).
- Consolidate findings from `handIn/ISSUES.md` into a prioritised backlog with severity labels.

**Exit Criteria**
- Baseline branch and tag exist; reference logs for build + short benchmark cycle captured.
- Issue backlog ordered (blocking correctness → structural → polish).
- Compatibility contract drafted (what keeps working during migration).

## Phase 1 – Stabilise Locks & Benchmarks
**Objectives**
- Resolve correctness and measurement bugs before moving files.
- Establish automated checks that guard future refactors.

**Key Tasks**
- Fix atomic misuse, per-thread node handling, fairness counters, and data races highlighted in `ISSUES.md`.
- Introduce unit/integration smoketests (C-level lock tests, Python harness validation, benchmark sanity assertions).
- Add a lightweight CI workflow that builds the shared library and runs the smoketests on the legacy layout.

**Exit Criteria**
- All blocking issues closed or tracked with clear owners.
- CI green on the legacy layout; benchmark metrics align with baseline tolerances.
- Documentation updated to note any breaking fixes or new configuration knobs.

## Phase 2 – Layout & Build Migration
**Objectives**
- Transition to an OSS-style layout while keeping builds reproducible.
- Provide tooling to bridge old and new entry points.

**Key Tasks**
- Finalise target structure (e.g. `src/`, `include/`, `python/`, `tools/`, `data/`, `docs/`, `examples/`, `outputs/`).
- Stand up a root build system (Makefile or CMake) compiling the shared library into `build/` or `dist/`.
- Move C sources/headers out of `handIn/framework/{src,include}` to their new homes; adjust include paths and build scripts.
- Ship a compatibility shim (wrapper Makefile/scripts) so existing commands continue to work during the transition.

**Exit Criteria**
- New layout builds and passes Phase 1 smoketests.
- Legacy commands issue deprecation warnings but function via the compatibility layer.
- `git status` clean aside from intentional moves; documentation outlines the new tree.

## Phase 3 – Python Tooling & Data Pipeline
**Objectives**
- Modernise benchmarking/plotting scripts and make paths configurable.
- Clarify data/plot output flows for local and Nebula runs.

**Key Tasks**
- Relocate Python modules into a dedicated package (e.g. `python/amp_benchmarks/` or `tools/benchmarks/`).
- Replace hard-coded `handIn/` references with `pathlib` + configuration (YAML/TOML or `config.py`) covering build artifacts, data dirs, and Nebula roots.
- Define canonical output directories (`outputs/data/`, `outputs/plots/`, `examples/data/` samples) and update `.gitignore`.
- Provide seeded sample datasets and fixtures for automated tests.

**Exit Criteria**
- Benchmark and plotting commands run end-to-end in the new structure with configurable directories.
- Compatibility layer forwards legacy invocations to the new package.
- Tests cover both real runs (short mode) and configuration parsing.

## Phase 4 – Packaging, Automation & CI Expansion
**Objectives**
- Deliver reproducible environments and automated validation for contributors.
- Prepare for external consumption (PyPI-style install or editable mode).

**Key Tasks**
- Publish dependency manifests (`requirements.txt` + lockfile or `pyproject.toml` with optional `setup.cfg`).
- Create CLI entry points (`python -m amp_benchmarks` or console scripts) with help text and defaults.
- Expand automated tests (Python unit tests, data validators, performance smoke thresholds).
- Extend CI to run on multiple platforms/compilers if feasible; cache build artifacts; surface benchmark deltas.
- Add linting/formatting (clang-format, black/ruff) with pre-commit hooks.

**Exit Criteria**
- `pip install -e .` (or equivalent) provisions the environment and exposes CLI tools.
- CI matrix green; benchmarks produce artefacts stored as build artifacts or uploaded docs.
- Contribution guidelines updated with testing and formatting expectations.

## Phase 5 – Documentation, Migration & Release
**Objectives**
- Communicate the new workflow, capture historical context, and finalise the transition.

**Key Tasks**
- Rewrite the README with quick-start guides, architecture overview, and examples.
- Move course-specific or historical material into `docs/history.md` (or similar) and keep Nebula instructions in dedicated docs.
- Draft a migration guide describing legacy → new commands, data locations, and release notes.
- Tag releases (`v0` legacy, `v1` restructured), publish changelog, and announce deprecation timeline for the compatibility layer.
- Monitor post-release issues; schedule removal of the shim once adoption is confirmed.

**Exit Criteria**
- Documentation live and consistent with the repository structure.
- Release artifacts published; old layout archived or removed per plan.
- Compatibility layer turned off (or scheduled) with user acknowledgement.

## Ongoing Considerations
- Track performance regressions by comparing key benchmarks against the Phase 0 baseline.
- Keep Nebula scripts updated but isolated under `examples/` or `docs/cluster/`.
- Review the backlog after each phase to reprioritise outstanding work and feed future releases.
