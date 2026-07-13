# Automated Database Management & ML Prediction Pipeline

A professional-grade quantitative pipeline for ingesting raw financial market data, cleaning it, storing it in a time-series database, engineering features, training machine-learning models, and generating low-latency trading signals.

## Architecture

```text
Raw market data (CSV / API / socket)
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

3. Generate sample data:

   ```powershell
   .\scripts\generate_sample_data.ps1
   ```

4. Build C++ pipeline:

   ```powershell
   .\scripts\build_cpp.ps1
   ```

5. Run ETL + DB load:

   ```powershell
   .\scripts\run_etl.ps1
   ```

6. Train ML model:

   ```powershell
   .\scripts\train_model.ps1
   ```

7. Run inference:

   ```powershell
   .\scripts\run_inference.ps1
   ```

## License

MIT
