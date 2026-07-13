"""Fetch real OHLCV data from Yahoo Finance and emit tick-like CSVs.

Yahoo Finance does not provide true tick data for free, so we download the
finest available granularity (1-minute when available, otherwise daily) and
expand each OHLCV bar into a small number of synthetic trade records that
preserve the bar's open, high, low, close and volume. This lets the existing
C++ ETL pipeline ingest real market prices while keeping its tick-oriented
schema unchanged.
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


def download_ohlcv(symbol: str, period: str = "7d", interval: str = "1m") -> pd.DataFrame:
    """Download OHLCV from Yahoo Finance."""
    logger.info("Downloading %s (%s, %s)", symbol, period, interval)
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
    """Expand each OHLCV bar into a few tick records.

    We emit up to 4 ticks per bar (open, high, low, close) and scale the
    bar volume across them. This is a pragmatic approximation of trade flow
    when true tick data is not available.
    """
    rows = []
    for _, bar in df.iterrows():
        volume = max(bar.get("volume", 0.0), 0.0)
        # Split volume across the four synthetic prints.
        sizes = [volume * 0.25] * 4
        ticks = [
            (bar["date"], bar["open"], sizes[0], "buy"),
            (bar["date"], bar["high"], sizes[1], "buy"),
            (bar["date"], bar["low"], sizes[2], "sell"),
            (bar["date"], bar["close"], sizes[3], "sell"),
        ]
        # Slightly stagger timestamps so the cleaner does not collapse them.
        for offset_ms, (ts, price, size, side) in enumerate(ticks):
            rows.append(
                {
                    "time": ts + pd.Timedelta(milliseconds=offset_ms * 10),
                    "symbol": symbol,
                    "price": round(price, 4),
                    "size": round(max(size, 1.0), 2),
                    "side": 1 if side == "buy" else -1,
                    "source": "yahoo_finance",
                }
            )
    return pd.DataFrame(rows)


def fetch(symbols, output_dir: Path, period: str = "7d", interval: str = "1m"):
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
    parser.add_argument("--period", default="7d")
    parser.add_argument("--interval", default="1m")
    args = parser.parse_args()

    fetch(args.symbols, Path(args.output_dir), args.period, args.interval)
    logger.info("Yahoo Finance fetch complete")


if __name__ == "__main__":
    main()
