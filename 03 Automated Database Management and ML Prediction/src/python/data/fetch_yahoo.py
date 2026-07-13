"""Fetch daily close prices from Yahoo Finance and emit tick-like CSVs.

Data is downloaded from the ticker history page using period=MAX and
interval=1d.  Because the C++ ETL pipeline is tick-oriented, each daily row
is emitted as a single synthetic trade record at the closing price.  This
provides the longest available historical daily close series for each symbol.
"""

import argparse
import logging
import sys
from pathlib import Path

import pandas as pd
import yfinance as yf

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)
logger = logging.getLogger(__name__)

# Top 10 S&P 500 tickers by market cap (approximate, stable set).
# Hyphens are replaced in table names by the sanitiser downstream.
DEFAULT_TOP_TEN = [
    "AAPL",
    "MSFT",
    "NVDA",
    "AMZN",
    "GOOGL",
    "META",
    "TSLA",
    "AVGO",
    "BRK-B",
    "JPM",
]


def download_ohlcv(symbol: str, period: str = "max", interval: str = "1d") -> pd.DataFrame:
    """Download daily OHLCV from Yahoo Finance with the requested period."""
    logger.info("Downloading %s (period=%s, interval=%s)", symbol, period, interval)
    ticker = yf.Ticker(symbol)
    df = ticker.history(period=period, interval=interval, auto_adjust=True)
    if df.empty:
        logger.warning("No data returned for %s", symbol)
        return df
    df = df.reset_index()
    # Standardise column names.
    df.columns = [c.lower().replace(" ", "_") for c in df.columns]
    # The datetime column may be named 'datetime' or 'date' depending on the
    # yfinance version; normalise it to 'date'.
    if "datetime" in df.columns and "date" not in df.columns:
        df = df.rename(columns={"datetime": "date"})
    # Remove timezone info for cleaner CSV parsing downstream.
    if pd.api.types.is_datetime64_any_dtype(df["date"]):
        df["date"] = df["date"].dt.tz_localize(None)
    return df


def bars_to_ticks(df: pd.DataFrame, symbol: str) -> pd.DataFrame:
    """Expand each daily OHLCV bar into a single close-price tick.

    The closing price is used as the representative trade price for the day.
    Volume is attached to that single synthetic print.  This matches the
    requirement to use daily closed prices from Yahoo Finance history.
    """
    rows = []
    for _, bar in df.iterrows():
        rows.append(
            {
                "time": bar["date"],
                "symbol": symbol,
                "price": round(bar["close"], 4),
                "size": round(max(bar.get("volume", 0.0), 1.0), 2),
                "side": 1,
                "source": "yahoo_finance",
            }
        )
    return pd.DataFrame(rows)


def fetch(symbols, output_dir: Path, period: str = "max", interval: str = "1d"):
    output_dir.mkdir(parents=True, exist_ok=True)
    for sym in symbols:
        df = download_ohlcv(sym, period=period, interval=interval)
        if df.empty:
            continue
        ticks = bars_to_ticks(df, sym)
        path = output_dir / f"{sym}.csv"
        ticks.to_csv(path, index=False, date_format="%Y-%m-%dT%H:%M:%S.%f")
        logger.info("Wrote %d ticks to %s", len(ticks), path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--symbols", nargs="+", default=DEFAULT_TOP_TEN)
    parser.add_argument("--output-dir", default="data/raw")
    parser.add_argument("--period", default="max")
    parser.add_argument("--interval", default="1d")
    args = parser.parse_args()

    fetch(args.symbols, Path(args.output_dir), args.period, args.interval)
    logger.info("Yahoo Finance fetch complete")


if __name__ == "__main__":
    main()
