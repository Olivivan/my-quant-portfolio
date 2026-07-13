"""Generate synthetic tick data for testing the pipeline."""

import argparse
import os
from datetime import datetime, timedelta
from pathlib import Path

import numpy as np
import pandas as pd


def generate_symbol(symbol: str, n_ticks: int = 10000, start_date: str = "2024-01-01") -> pd.DataFrame:
    rng = np.random.default_rng(seed=hash(symbol) % 2**32)
    start = datetime.strptime(start_date, "%Y-%m-%d") + timedelta(hours=9, minutes=30)
    times = [start + timedelta(seconds=i) for i in range(n_ticks)]

    price = 100.0 + rng.random() * 100.0
    prices = []
    sizes = []
    sides = []
    for _ in range(n_ticks):
        price *= 1.0 + (rng.random() - 0.5) * 0.002
        prices.append(price)
        sizes.append(round(rng.random() * 1000 + 100, 2))
        sides.append(1 if rng.random() > 0.5 else -1)

    # Inject a few outliers for testing
    outlier_idx = rng.integers(0, n_ticks, size=5)
    for idx in outlier_idx:
        prices[idx] *= 1.5

    df = pd.DataFrame({
        "time": times,
        "symbol": symbol,
        "price": prices,
        "size": sizes,
        "side": sides,
        "source": "synthetic",
    })
    return df


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="data/raw")
    parser.add_argument("--symbols", nargs="+", default=["AAPL", "MSFT", "GOOGL", "TSLA"])
    parser.add_argument("--n-ticks", type=int, default=10000)
    args = parser.parse_args()

    out = Path(args.output)
    out.mkdir(parents=True, exist_ok=True)
    for sym in args.symbols:
        df = generate_symbol(sym, args.n_ticks)
        path = out / f"{sym}.csv"
        df.to_csv(path, index=False)
        print(f"Generated {path} with {len(df)} rows")


if __name__ == "__main__":
    main()
