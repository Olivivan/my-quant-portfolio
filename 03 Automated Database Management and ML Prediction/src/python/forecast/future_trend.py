"""Train a simple ML trend model on historical daily closes and forecast prices
through a target date (default 2030-12-31).

The model regresses log(close) on time using scikit-learn LinearRegression and
blends in recent momentum so the forecast respects the latest price action.
Output is written to the `forecasts` table and can be plotted by
src/python/visualization/plot_predictions.py.

Usage:
    .venv\Scripts\Activate.ps1
    $env:PYTHONPATH = 'src/python'
    python src/python/forecast/future_trend.py --end-date 2030-12-31
"""
from __future__ import annotations

import argparse
import logging
import sys
from datetime import datetime
from pathlib import Path

import numpy as np
import pandas as pd
import psycopg2
from sklearn.linear_model import LinearRegression

PROJECT_ROOT = Path(__file__).resolve().parents[2]
SRC_PYTHON = PROJECT_ROOT / "src" / "python"
if str(SRC_PYTHON) not in sys.path:
    sys.path.insert(0, str(SRC_PYTHON))

from common.config import Config

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
)
logger = logging.getLogger(__name__)


def ensure_forecast_table(conn) -> None:
    """Create the forecasts table if it does not exist."""
    with conn.cursor() as cur:
        cur.execute(
            """
            CREATE TABLE IF NOT EXISTS forecasts (
                time        timestamp with time zone NOT NULL,
                symbol      text                     NOT NULL,
                model_name  text                     NOT NULL,
                forecast    double precision         NOT NULL,
                lower_95    double precision,
                upper_95    double precision,
                PRIMARY KEY (time, symbol, model_name)
            );
            CREATE INDEX IF NOT EXISTS idx_forecasts_symbol_time
                ON forecasts (symbol, time DESC);
            """
        )
    conn.commit()


def load_daily_closes(cfg: Config) -> pd.DataFrame:
    """Load the latest daily close per symbol from bars_1m."""
    d = cfg.raw["database"]
    query = """
        SELECT DISTINCT ON (symbol, DATE(time AT TIME ZONE 'UTC'))
               time AT TIME ZONE 'UTC' AS time,
               symbol,
               close
        FROM bars_1m
        ORDER BY symbol, DATE(time AT TIME ZONE 'UTC'), time DESC;
    """
    df = pd.read_sql(query, cfg.db_uri())
    df["time"] = pd.to_datetime(df["time"]).dt.tz_localize(None)
    df = df.sort_values(["symbol", "time"]).reset_index(drop=True)
    return df


def forecast_symbol(
    df: pd.DataFrame,
    symbol: str,
    end_date: str,
    momentum_window: int = 252,
    trend_weight: float = 0.7,
) -> pd.DataFrame:
    """Fit a log-price trend model and project daily closes up to end_date."""
    s = df[df["symbol"] == symbol].copy().sort_values("time")
    if s.empty:
        return pd.DataFrame()

    s["ordinal"] = s["time"].map(datetime.toordinal)
    X = s[["ordinal"]].values
    y = np.log(s["close"].values)

    model = LinearRegression()
    model.fit(X, y)

    last_date = s["time"].iloc[-1]
    last_close = s["close"].iloc[-1]
    last_ordinal = datetime.toordinal(last_date)

    # Recent momentum (annualized log return over the trailing window).
    if len(s) >= momentum_window:
        recent_return = y[-1] - y[-momentum_window]
    else:
        recent_return = y[-1] - y[0]
    # Annualize by the actual number of years in the window.
    years = max((last_ordinal - s["ordinal"].iloc[-min(len(s), momentum_window)]) / 365.25, 1.0)
    momentum_drift = recent_return / years

    # Blend trend drift with recent momentum.
    trend_drift = model.coef_[0] * 365.25  # annualized log drift from trend
    blended_drift = trend_weight * trend_drift + (1 - trend_weight) * momentum_drift

    # Residual std around the trend -> confidence bands.
    y_pred = model.predict(X)
    resid_std = np.std(y - y_pred)

    future_dates = pd.date_range(start=last_date + pd.Timedelta(days=1), end=end_date, freq="D")
    future_ordinals = np.array([datetime.toordinal(d) for d in future_dates])
    days_ahead = future_ordinals - last_ordinal

    # Log-price forecast: start from last log price and add blended daily drift.
    log_forecast = np.log(last_close) + blended_drift * (days_ahead / 365.25)
    forecast = np.exp(log_forecast)
    lower = np.exp(log_forecast - 1.96 * resid_std * np.sqrt(days_ahead / 365.25))
    upper = np.exp(log_forecast + 1.96 * resid_std * np.sqrt(days_ahead / 365.25))

    return pd.DataFrame({
        "time": future_dates,
        "symbol": symbol,
        "model_name": "log_trend_blend",
        "forecast": forecast,
        "lower_95": lower,
        "upper_95": upper,
    })


def save_forecasts(conn, forecasts: pd.DataFrame) -> None:
    """Upsert forecast rows into the forecasts table."""
    if forecasts.empty:
        return
    with conn.cursor() as cur:
        for _, row in forecasts.iterrows():
            cur.execute(
                """
                INSERT INTO forecasts (time, symbol, model_name, forecast, lower_95, upper_95)
                VALUES (%s, %s, %s, %s, %s, %s)
                ON CONFLICT (time, symbol, model_name) DO UPDATE
                SET forecast = EXCLUDED.forecast,
                    lower_95 = EXCLUDED.lower_95,
                    upper_95 = EXCLUDED.upper_95;
                """,
                (
                    row["time"],
                    row["symbol"],
                    row["model_name"],
                    row["forecast"],
                    row["lower_95"],
                    row["upper_95"],
                ),
            )
    conn.commit()


def main() -> None:
    parser = argparse.ArgumentParser(description="Forecast future price trends.")
    parser.add_argument("--config", default="config/pipeline.json")
    parser.add_argument("--end-date", default="2030-12-31")
    parser.add_argument("--momentum-window", type=int, default=252)
    parser.add_argument("--trend-weight", type=float, default=0.7)
    args = parser.parse_args()

    cfg = Config(args.config)
    logger.info("Loading historical daily closes...")
    closes = load_daily_closes(cfg)

    d = cfg.raw["database"]
    conn = psycopg2.connect(
        host=d["host"],
        port=d["port"],
        dbname=d["dbname"],
        user=d["user"],
        password=d["password"],
    )
    ensure_forecast_table(conn)

    all_forecasts: list[pd.DataFrame] = []
    for symbol in cfg.symbols:
        logger.info("Forecasting %s through %s", symbol, args.end_date)
        f = forecast_symbol(
            closes,
            symbol,
            args.end_date,
            momentum_window=args.momentum_window,
            trend_weight=args.trend_weight,
        )
        if not f.empty:
            all_forecasts.append(f)

    if all_forecasts:
        forecasts = pd.concat(all_forecasts, ignore_index=True)
        save_forecasts(conn, forecasts)
        logger.info("Saved %d forecast rows", len(forecasts))
    else:
        logger.warning("No forecasts generated")

    conn.close()


if __name__ == "__main__":
    main()
