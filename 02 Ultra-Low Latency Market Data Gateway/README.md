# Ultra-Low Latency Market Data Gateway

Modular C++ project scaffold for building a high-performance market data gateway.

## Highlights

- Hierarchical CMake layout with isolated modules
- Automated dependency fetching (Catch2) via `FetchContent`
- Two-stage parsing architecture in feeds (`Structural Scan` -> `Data Access`)
- Runtime SIMD dispatch (AVX-512 -> AVX2 -> scalar) for structural scan
- Structural index layout for O(1) field lookup in data access
- PMR monotonic parse context for zero-allocation hot path parsing
- SBE flyweight overlays for direct raw-buffer binary access
- Alignment-aware SIMD paths using `std::assume_aligned` compiler hints
- Consteval tag lookup tables replacing runtime switch/map dispatch
- Compile-time FNV-1a hashing for O(1) message type routing jump tables
- Slowpath removal and branch reduction using `[[likely]]`/`[[unlikely]]`
- Socket-layer hardware timestamping configuration (`SO_TIMESTAMPING`)
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
	- Linux socket timestamping configuration for NIC ingress latency capture
- `src/feeds`: feed normalization module
	- Stage 1: structural scanner with runtime feature detection and function-pointer dispatch
	  - AVX-512 kernel: 64-byte cycle delimiter scan
	  - AVX2 kernel: 32-byte cycle delimiter scan
	  - Alignment hints: `std::assume_aligned` fast paths for aligned load instructions
	  - Scalar kernel: portable fallback
	  - PMR marker storage: `std::pmr::vector` on `std::pmr::monotonic_buffer_resource`
	  - Structural index: precomputed hash slots for constant-time key lookup
	  - Consteval tags: compile-time binary lookup arrays for known fields
	  - Branch hints on hot loops with extracted slowpath error handlers
	- Stage 2: data access view (typed field lookup without reparsing)
- `src/sbe`: binary encoding/decoding module
	- Flyweight wrappers over packed SBE structs
	- Direct struct overlays on network buffers (no copy)
- `src/gateway`: pipeline orchestration module
	- Message type router using compile-time hash constants and switch-based jump table
- `tests`: unit tests (Catch2)
