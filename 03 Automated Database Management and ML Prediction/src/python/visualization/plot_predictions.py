r"""Plot raw historical close prices overlaid with model predictions.

Usage:
    .venv\Scripts\Activate.ps1
    $env:PYTHONPATH = 'src/python'
    python src/python/visualization/plot_predictions.py --show

Use --normalize to rebase every ticker to 100 so they fit on the same scale.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import pandas as pd

# Allow importing from src/python/common when running directly.
PROJECT_ROOT = Path(__file__).resolve().parents[2]
SRC_PYTHON = PROJECT_ROOT / "src" / "python"
if str(SRC_PYTHON) not in sys.path:
    sys.path.insert(0, str(SRC_PYTHON))

from common.config import Config


def load_forecasts(cfg: Config) -> pd.DataFrame:
    """Load ML-generated future price forecasts from the forecasts table."""
    query = """
        SELECT time AT TIME ZONE 'UTC' AS time,
               symbol,
               model_name,
               forecast
        FROM forecasts
        ORDER BY symbol, time
    """
    df = pd.read_sql(query, cfg.db_uri())
    df["time"] = pd.to_datetime(df["time"])
    return df


def load_raw_prices(cfg: Config) -> pd.DataFrame:
    """Load raw tick CSVs emitted by the Yahoo fetcher."""
    raw_dir = Path(cfg.raw["ingestion"]["raw_data_dir"])
    frames: list[pd.DataFrame] = []
    for symbol in cfg.symbols:
        path = raw_dir / f"{symbol}.csv"
        if not path.exists():
            print(f"Warning: raw data not found for {symbol} at {path}", file=sys.stderr)
            continue
        df = pd.read_csv(path, parse_dates=["time"])
        df = df[df["symbol"] == symbol].copy()
        df = df.sort_values("time")
        df["time"] = df["time"].dt.tz_localize(None)
        df = df.rename(columns={"price": "close"})
        frames.append(df[["time", "symbol", "close"]])
    return pd.concat(frames, ignore_index=True)


def load_signals(cfg: Config, model_name: str) -> pd.DataFrame:
    """Load prediction signals from PostgreSQL for a given model."""
    query = """
        SELECT time AT TIME ZONE 'UTC' AS time,
               symbol,
               prediction,
               probability
        FROM signals
        WHERE model_name = %s
        ORDER BY time, symbol
    """
    df = pd.read_sql(query, cfg.db_uri(), params=(model_name,))
    df["time"] = pd.to_datetime(df["time"])
    # Defensive: remove duplicate predictions for the same symbol/date.
    df["date"] = df["time"].dt.normalize()
    df = df.drop_duplicates(subset=["symbol", "date"])
    return df


def add_prediction_overlay(
    ax: plt.Axes,
    symbol: str,
    color,
    price_series: pd.Series,
    forecasts: pd.DataFrame,
) -> None:
    """Plot the ML-generated future price trend as a dashed line."""
    f = forecasts[forecasts["symbol"] == symbol].copy().sort_values("time")
    if f.empty:
        return

    ax.plot(
        f["time"],
        f["forecast"],
        color=color,
        linestyle="--",
        linewidth=1.5,
        alpha=0.9,
        label=f"{symbol} forecast",
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Line chart of raw historical prices + model predictions."
    )
    parser.add_argument(
        "--config", default="config/pipeline.json", help="Path to pipeline config"
    )
    parser.add_argument(
        "--model-name",
        default=None,
        help="Model name stored in the signals table (default: config.inference.model_name)",
    )
    parser.add_argument(
        "--output",
        default="data/plots/predictions.png",
        help="Output image path",
    )
    parser.add_argument(
        "--show", action="store_true", help="Show the plot interactively"
    )
    parser.add_argument(
        "--normalize",
        action="store_true",
        help="Rebase every ticker to its first close (base = 100) for comparability",
    )
    parser.add_argument(
        "--log-y", action="store_true", help="Use a log scale for the price axis"
    )
    parser.add_argument(
        "--start-date",
        default="2010-01-01",
        help="Start date for the chart (YYYY-MM-DD)",
    )
    parser.add_argument(
        "--end-date",
        default="2026-12-31",
        help="End date for the chart (YYYY-MM-DD)",
    )
    args = parser.parse_args()

    cfg = Config(args.config)

    prices = load_raw_prices(cfg)
    if prices.empty:
        raise RuntimeError("No raw price data found.")

    forecasts = load_forecasts(cfg)
    if forecasts.empty:
        raise RuntimeError(
            "No forecasts found. Run src/python/forecast/future_trend.py first."
        )

    start = pd.Timestamp(args.start_date)
    end = pd.Timestamp(args.end_date)
    prices = prices[(prices["time"] >= start) & (prices["time"] <= end)].copy()
    forecasts = forecasts[(forecasts["time"] >= start) & (forecasts["time"] <= end)].copy()

    if args.normalize:
        prices["close"] = prices.groupby("symbol")["close"].transform(
            lambda x: x / x.iloc[0] * 100
        )

    fig, ax = plt.subplots(figsize=(18, 9))
    symbols = sorted(prices["symbol"].unique())
    cmap = plt.cm.tab10

    for i, symbol in enumerate(symbols):
        color = cmap(i % 10)
        s_prices = prices[prices["symbol"] == symbol].set_index("time").sort_index()

        # Solid line for raw historical prices.
        ax.plot(
            s_prices.index,
            s_prices["close"],
            color=color,
            linewidth=1.2,
            label=f"{symbol} price",
        )

        # Dashed ML-generated future price trend.
        add_prediction_overlay(ax, symbol, color, s_prices["close"], forecasts)

    ax.set_title("Raw Historical Prices and Model Predictions")
    ax.set_xlabel("Date")
    ylabel = "Normalized Price (base = 100)" if args.normalize else "Price"
    ax.set_ylabel(ylabel)
    if args.log_y:
        ax.set_yscale("log")
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper left", fontsize="small", ncol=2)

    ax.xaxis.set_major_locator(mdates.YearLocator())
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%Y"))
    fig.autofmt_xdate()

    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.tight_layout()
    fig.savefig(out_path, dpi=150)
    print(f"Saved plot to {out_path.resolve()}")

    if args.show:
        plt.show()


if __name__ == "__main__":
    main()
