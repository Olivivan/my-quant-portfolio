# Limit Order Book Matching Engine

This project implements a high-performance limit order book matching engine in modern C++20.
It is designed as a small reference system for exploring:

- intrusive doubly linked order lists
- price-level aggregation and FIFO execution
- pool-based object reuse to avoid per-order allocation in the hot path
- a partitioned multi-threaded matching kernel for parallel ingestion and matching

## What it does

The engine accepts incoming orders and:

1. Matches aggressive orders against resting liquidity on the opposite side.
2. Leaves any remaining quantity as a resting limit order.
3. Supports cancellation of active orders.
4. Runs a randomized benchmark workload to measure throughput.

## Project structure

- Order.hpp: order node definition
- MemoryPool.hpp: preallocated object pool
- OrderBook.hpp: order book and worker interfaces
- OrderBook.cpp: matching engine implementation
- main.cpp: randomized benchmark harness

## Build

Use a C++20 compiler with pthread support:

```bash
g++ -std=c++20 -O3 -DNDEBUG -pthread main.cpp OrderBook.cpp -o lob_benchmark.exe
```

## Run

```bash
./lob_benchmark.exe
```

## Notes

This is a research and benchmark-oriented implementation rather than a full exchange production stack. It is intended to demonstrate architecture ideas for low-latency matching engines in C++.
