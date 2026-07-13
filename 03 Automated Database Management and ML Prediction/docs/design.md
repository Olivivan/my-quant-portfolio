# Design Document

## Goals

1. Fetch raw financial data, clean it, impute missing ticks, and store clean time-series data.
2. Engineer features for short-term price movement prediction.
3. Train ML models with rigorous cross-validation and overfitting controls.
4. Export models to ONNX and run low-latency inference in C++.
5. Minimize licensing, operational, and maintenance costs while maximizing throughput.

## Why Hybrid C++/Python

A pure C++ stack would force model training in C++. The available C++ ML libraries (mlpack, dlib, Shark) have smaller communities, weaker hyperparameter tooling, and slower experimentation cycles. The hybrid keeps the hot path in C++ and the research/ML loop in Python.

## Cost Optimization

| Decision | Cost Impact |
| -------- | ----------- |
| TimescaleDB via Docker | Avoids proprietary TSDB licensing |
| Redis via Docker | Avoids managed cache fees |
| Parquet/Arrow cold store | Free columnar format, cheap cloud object storage later |
| ONNX Runtime C++ inference | No per-inference fees, low latency |
| CMake + vcpkg | No paid build tools |
| Open-source Python ML stack | No ML platform fees |

## Parallelism Strategy

- **Ingestion**: C++ thread pool parses multiple files/symbols concurrently.
- **Cleaning**: Per-symbol worker threads detect outliers and impute missing ticks.
- **Feature engineering**: SIMD-friendly rolling-window calculations; partitioned by symbol.
- **ML training**: `joblib`/`loky` for hyperparameter search; LightGBM/XGBoost native multi-threading.
- **Inference**: Batched ONNX Runtime sessions with intra/inter op thread pools.

## Data Quality & Overfitting Controls

- Outlier detection using modified Z-score and winsorization.
- Missing-tick imputation using forward-fill and interpolation.
- Purged k-fold cross-validation with embargo to prevent leakage.
- Feature selection via permutation importance and recursive feature elimination.
- Walk-forward validation for time-series models.
- Profit-based secondary scoring with transaction cost adjustment.

## Schema

See [schema.sql](schema.sql) for hypertable definitions:

- `ticks`: raw trade/quote ticks
- `bars_1m`: OHLCV bars
- `features`: engineered features per bar
- `signals`: model predictions
- `model_registry`: trained model metadata
