# Automated Database Management & ML Prediction Pipeline

A professional-grade quantitative pipeline for ingesting raw financial market data, cleaning it, storing it in a time-series database, engineering features, training machine-learning models, and generating low-latency trading signals.

## Architecture

```text
Yahoo Finance OHLCV
        │
        ▼
┌─────────────────────────────┐
│   Python fetch_yahoo.py     │  <- real 1-minute data, top-10 US equities
└─────────────────────────────┘
        │
        ▼
Tick-like CSV per symbol
        │
        ▼
┌─────────────────────────────┐
│  C++ Ingestion + ETL Engine │  <- lock-free queues, multi-threaded, SIMD-ready
└─────────────────────────────┘
        │
    ┌───┴───┐
    ▼       ▼
TimescaleDB   Parquet cold store
(PostgreSQL)  (Apache Arrow)
    │
    ▼
C++ Feature Engineering
    │
    ▼
Redis hot cache  ──►  Python ML Training  ──►  ONNX model
    ▲                                               │
    └──────── C++ ONNX Inference Engine ◄───────────┘
                            │
                            ▼
                  Trading signals / execution

Top-10 universe: AAPL, MSFT, NVDA, AMZN, GOOGL, META, TSLA, AVGO, BRK-B, JPM.

Ticks are stored in per-symbol tables (`ticks_<symbol>`).  ETL, feature
engineering and inference are run **one ticker at a time** so predictions are
produced sequentially after each symbol's features are built.
```

## Technology Choices

| Layer | Technology | Reason |
| ----- | ---------- | ------ |
| Ingestion / ETL / Features | C++23 | Maximum throughput, deterministic latency, zero runtime licensing |
| Time-series database | PostgreSQL + TimescaleDB (Docker) | Free, production-proven, hypertables for ticks/OHLCV |
| Cold / research store | Apache Parquet + Arrow | Free columnar storage, ML-native format |
| Hot feature cache | Redis (Docker) | Sub-ms serving, free |
| ML training | Python + LightGBM / XGBoost / scikit-learn | Best tabular ML ecosystem, free |
| ML inference | C++ ONNX Runtime | Python-trained models executed at C++ speed |
| Build | CMake + vcpkg | Standard, portable, dependency isolation |

This hybrid minimizes tool licensing and maintenance costs without sacrificing performance. Parallelism is exploited in C++ via thread pools and in Python via `joblib` / `concurrent.futures`.

## Repository Layout

```text
.
├── config/                 # JSON configuration
├── docker/                 # Docker Compose for TimescaleDB + Redis
├── docs/                   # Design and operations docs
├── scripts/                # PowerShell setup and run scripts
├── src/
│   ├── cpp/                # C++ ETL, DB, feature, inference code
│   └── python/             # Python feature/ML training code
├── tests/                  # Unit tests and integration tests
├── models/                 # Trained ONNX models (gitignored)
└── data/                   # Raw / clean / parquet data (gitignored)
```

## Quick Start

1. Install prerequisites (see [docs/install.md](docs/install.md)).

2. Start infrastructure:

   ```powershell
   .\scripts\start_infra.ps1
   ```

   If Docker is unavailable, this script falls back to a local PostgreSQL 16
   instance and initializes the `quantdb` database. TimescaleDB hypertables are
   created automatically when the extension is installed; otherwise plain
   PostgreSQL tables are used.

3. (Optional) Build C++ pipeline:

   ```powershell
   .\scripts\build_cpp.ps1
   ```

4. Run the real-data Yahoo Finance pipeline end-to-end:

   ```powershell
   .\scripts\run_pipeline_yahoo.ps1
   ```

   This script: fetches 7-day 1-minute OHLCV for the top-10 universe from
   Yahoo Finance, trains a LightGBM direction model, exports it to ONNX, then
   loops through each symbol running ETL → features → inference one ticker at
   a time.

5. Run a single stage for one symbol:

   ```powershell
   .\scripts\run_etl.ps1 -Symbol AAPL
   .\scripts\run_features.ps1 -Symbol AAPL
   .\scripts\run_inference.ps1 -Symbol AAPL
   ```

## License

MIT
