# High-Frequency Trading Core (HFT Core)

![Status](https://img.shields.io/badge/status-portfolio%20prototype-yellow) ![Language](https://img.shields.io/badge/language-C%2B%2B17-blue) ![CI](https://img.shields.io/badge/ci-github%20actions-informational)

## What This Is

HFT Core is a C++17 portfolio project that implements a deterministic limit-order-book matching engine with price-time priority, order management operations, and a runnable simulation and benchmark mode.

It is not marketed as a production trading engine. The current focus is correctness, clarity, and engineering process quality for recruiter review.

## Implemented Features

- Price-time priority matching for limit orders
- Cross-book execution (aggressive orders sweep opposite-side liquidity)
- Partial fill handling with residual book insertion
- Order cancellation by id
- Order replacement by id (price, quantity, timestamp)
- Input validation (id, price, quantity, timestamp, duplicate active id rejection)
- Deterministic simulation mode with human-readable top-of-book snapshots
- Deterministic benchmark mode with throughput output

## Architecture

- `hft_main` (executable): runs deterministic simulation or benchmark from CLI
- `hft_core_lib` (header-only INTERFACE target): matching logic and order book management
- `hft_render_lib` (header-only INTERFACE target): simple runtime rendering/log helpers
- `hft_tests` (test executable): Catch2-based unit tests for matching and management behavior

## Build And Run

### Requirements

- C++17 compiler
- CMake 3.16+
- Git (Catch2 is fetched by CMake)

### Configure and build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Run simulation

```bash
./build/bin/hft_main --simulate
```

### Run benchmark

```bash
./build/bin/hft_main --benchmark
```

## Testing

Tests are located in `tests/tests_main.cpp` and currently cover:

- Price priority and time priority
- Partial fills and remainder persistence
- Equal timestamp deterministic behavior
- Cancel and replace flows
- Invalid-order rejection
- Crossed-book sweep behavior

Run tests:

```bash
ctest --test-dir build -C Release --output-on-failure
```

## Static Analysis

- Compiler warnings are enabled by default (`/W4 /permissive-` on MSVC, `-Wall -Wextra -Wpedantic` otherwise)
- Optional `clang-tidy` integration is available with:

```bash
cmake -S . -B build -DHFT_ENABLE_CLANG_TIDY=ON
```

- CI also runs `cppcheck` for baseline static analysis.

## CI (Push + Pull Request)

GitHub Actions workflow: `.github/workflows/ci.yml`

- Builds on `ubuntu-latest` and `windows-latest`
- Runs tests with CTest
- Runs `cppcheck` static analysis
- Runs a Linux benchmark threshold gate using `scripts/check_benchmark_threshold.py`
- Uploads Linux benchmark output and a generated history-row CSV as artifacts

## Performance Snapshot (Reproducible)

The command below was used locally:

```bash
hft_main --benchmark
```

Benchmark settings in current code:

- `200000` synthetic limit orders
- Alternating buy/sell flow
- Fixed deterministic price pattern and quantity range

Measured output (local run):

- `benchmark_orders=200000`
- `benchmark_elapsed_ms=21805.134`
- `benchmark_orders_per_sec=9172`

Machine and toolchain used for this snapshot:

- CPU: 13th Gen Intel Core i9-13900K (24 cores / 32 threads)
- RAM: 127.7 GB
- OS: Microsoft Windows 11 Pro (10.0.26200, 64-bit)
- Compiler: `g++.exe (MSYS2) 16.1.0`

Notes:

- This benchmark is single-run and single-process.
- Numbers should be treated as a baseline for regression tracking, not as a low-latency production claim.
- Benchmark tracking files are stored in `benchmark/history.csv` and `benchmark/thresholds.json`.

## Roadmap

- Add order modify semantics aligned with specific venue-style rules
- Add latency histograms and percentile reporting
- Expand tests with randomized property checks and replay fixtures
- Add richer market data and trade event stream output

## License

All rights reserved. This project is proprietary.
