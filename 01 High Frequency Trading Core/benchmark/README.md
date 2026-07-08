# Benchmark Tracking

This folder stores benchmark baselines and the threshold used in CI.

## Files

- history.csv: manually curated benchmark history for trend tracking
- thresholds.json: minimum throughput required by CI

## CI behavior

The workflow runs benchmark mode on Linux and validates output with scripts/check_benchmark_threshold.py.

If benchmark_orders_per_sec is below min_orders_per_sec in thresholds.json, CI fails.

## Updating history

After validating a new stable baseline, append a new row to history.csv with date, environment, and results.
