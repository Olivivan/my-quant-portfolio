# My Quant Portfolio

My Quant Portfolio is a multi-project repository focused on low-latency trading systems, market data infrastructure, quantitative research, and machine-learning-driven analytics. The codebase is organized as a practical engineering portfolio: each project explores a distinct systems problem in modern C++ or Python, with emphasis on determinism, performance, and clear architecture.

## Overview

This repository combines several independent but related projects:

- high-frequency trading engine design
- ultra-low-latency market data processing
- data engineering and ML prediction pipelines
- event-driven backtesting and risk analytics
- supporting experiments in time-series analytics, pricing, and core low-latency techniques

The main audience is reviewers, collaborators, and hiring teams evaluating systems design, performance-oriented C++ work, and end-to-end quantitative engineering.

## Featured Projects

### 1. High Frequency Trading Core

Location: [01 High Frequency Trading Core](./01%20High%20Frequency%20Trading%20Core/)

A deterministic C++ trading core centered on a limit-order-book matching engine with price-time priority, order lifecycle management, simulation mode, benchmark mode, and test coverage.

Highlights:

- price-time priority matching
- partial fills, cancel, and replace flows
- deterministic simulation and benchmark runs
- CMake-based build and Catch2 tests

Documentation: [01 High Frequency Trading Core/README.md](./01%20High%20Frequency%20Trading%20Core/README.md)

### 2. Ultra-Low Latency Market Data Gateway

Location: [02 Ultra-Low Latency Market Data Gateway](./02%20Ultra-Low%20Latency%20Market%20Data%20Gateway/)

A C++20 market data ingress engine designed around deterministic critical paths, SIMD parsing, lock-free deferred work queues, and low-jitter routing for high-frequency environments.

Highlights:

- two-stage parsing pipeline
- compile-time message routing
- NUMA and thread-affinity aware design
- benchmark suite with latency-oriented metrics

Documentation: [02 Ultra-Low Latency Market Data Gateway/README.md](./02%20Ultra-Low%20Latency%20Market%20Data%20Gateway/README.md)

### 3. Automated Database Management and ML Prediction

Location: [03 Automated Database Management and ML Prediction](./03%20Automated%20Database%20Management%20and%20ML%20Prediction/)

An end-to-end quantitative data pipeline that ingests market data, stores it in a time-series database, engineers features, trains machine-learning models, and serves C++ inference output.

Highlights:

- C++ ETL and feature engineering
- PostgreSQL or TimescaleDB-backed market data storage
- Redis hot cache and Parquet cold storage
- Python model training with ONNX export for C++ inference

Documentation: [03 Automated Database Management and ML Prediction/README.md](./03%20Automated%20Database%20Management%20and%20ML%20Prediction/README.md)

### 4. Limit Order Book Matching Engine

Location: [Limit Order Book](./Limit%20Order%20Book/)

A focused matching-engine implementation that explores intrusive order lists, price-level aggregation, object pooling, and benchmark-oriented execution.

Highlights:

- FIFO execution within price levels
- pool-based object reuse on the hot path
- cancellation support
- randomized benchmark harness

Documentation: [Limit Order Book/README.md](./Limit%20Order%20Book/README.md)

## Additional Projects

The repository also includes smaller systems and experiments that support the broader portfolio:

- [Event-Driven Backtester](./Event-Driven%20Backtester/) for event-based simulation and risk analytics components
- [Multi-Threaded Monte Carlo Pricing Engine](./Multi-Threaded%20Monte%20Carlo%20Pricing%20Engine/) for parallel derivatives pricing experiments
- [Time-Series Analytics Library](./Time-Series%20Analytics%20Library/) for reusable analytics and financial vector abstractions
- [High Frequency Infrastructure](./High%20Frequency%20Infrastructure/) for infrastructure-oriented low-latency concepts
- [Key Low Latency Techniques](./Key%20Low%20Latency%20Techniques/) for targeted performance-focused implementations
- [Practice](./Practice/) for smaller C++ learning exercises and standalone programs

## Technology Stack

- C++17, C++20, and C++23 for performance-critical systems work
- Python for data workflows, research tooling, and model training
- CMake for native build orchestration
- Catch2 and benchmark tooling for validation and performance measurement
- PostgreSQL, TimescaleDB, Redis, Arrow, and Parquet for data infrastructure
- ONNX Runtime for production-style model inference in C++

## Repository Structure

```text
.
├── 01 High Frequency Trading Core/
├── 02 Ultra-Low Latency Market Data Gateway/
├── 03 Automated Database Management and ML Prediction/
├── Event-Driven Backtester/
├── High Frequency Infrastructure/
├── Key Low Latency Techniques/
├── Limit Order Book/
├── Multi-Threaded Monte Carlo Pricing Engine/
├── Practice/
└── Time-Series Analytics Library/
```

## Getting Started

Most projects are self-contained and can be built or run independently from their own directories.

Typical workflow:

1. Choose a project from the sections above.
2. Open that project folder and read its local README.
3. Configure dependencies for that project.
4. Build and run its tests or benchmark suite.

For CMake-based projects, the standard flow is typically:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Some projects also include Python dependencies, Docker services, or project-specific scripts. Use each subproject README as the source of truth for exact setup steps.

## Engineering Focus

This portfolio emphasizes:

- deterministic behavior over opaque abstraction
- low-latency data paths and memory-aware design
- testable, modular C++ architecture
- reproducible benchmarks and measurable performance claims
- practical integration between systems programming and quantitative workflows

## Notes

- Several directories contain local build artifacts such as `build/` and `build-ninja/` folders.
- The projects are not presented as a single integrated application; they are independent portfolio pieces with overlapping quantitative systems themes.
- Licensing may differ by subproject. Check each project directory before reuse or distribution.

## Contact and Use

This repository is best read as an engineering portfolio covering trading infrastructure, low-latency systems, and quantitative tooling. If you are reviewing it for collaboration or hiring, start with the featured projects above and then drill into the supporting experiments relevant to your use case.
