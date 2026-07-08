# Ultra-Low Latency Market Data Gateway

Modular C++ project scaffold for building a high-performance market data gateway.

## Highlights

- Hierarchical CMake layout with isolated modules
- Automated dependency fetching (Catch2) via `FetchContent`
- Two-stage parsing architecture in feeds (`Structural Scan` -> `Data Access`)
- Baseline gateway orchestration and test harness

## Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Structure

- `apps/gateway`: executable entrypoint
- `src/common`: shared low-level components
- `src/network`: network ingestion module
- `src/feeds`: feed normalization module
	- Stage 1: structural scanner (AVX2 scan of SOH and `=` delimiters)
	- Stage 2: data access view (typed field lookup without reparsing)
- `src/gateway`: pipeline orchestration module
- `tests`: unit tests (Catch2)
