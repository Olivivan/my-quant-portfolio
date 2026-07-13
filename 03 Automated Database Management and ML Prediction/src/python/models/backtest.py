"""Simple cost-aware backtest using stored signals."""

import argparse
import logging

import numpy as np
import pandas as pd
import psycopg2
from common.config import Config

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


def backtest(symbol: str, config_path: str):
    cfg = Config(config_path)
    d = cfg.raw["database"]
    conn = psycopg2.connect(
        host=d["host"],
        port=d["port"],
        dbname=d["dbname"],
        user=d["user"],
        password=d["password"],
    )

    bars = pd.read_sql(
        "SELECT time, close FROM bars_1m WHERE symbol = %s ORDER BY time",
        conn,
        params=(symbol,),
        parse_dates=["time"],
        index_col="time",
    )
    signals = pd.read_sql(
        "SELECT time, prediction, probability FROM signals WHERE symbol = %s AND model_name = %s ORDER BY time",
        conn,
        params=(symbol, "lgbm_direction"),
        parse_dates=["time"],
        index_col="time",
    )
    conn.close()

    df = bars.join(signals)
    df["returns"] = df["close"].pct_change().shift(-1)
    df["strategy_returns"] = np.where(df["prediction"] == 1, df["returns"], np.where(df["prediction"] == -1, -df["returns"], 0))

    cost_bps = cfg.ml_cfg.get("cost_per_trade_bps", 1.0)
    trades = df["prediction"].diff().fillna(0).abs()
    df["strategy_returns"] -= trades * cost_bps / 10000

    total_return = (1 + df["strategy_returns"].fillna(0)).prod() - 1
    sharpe = df["strategy_returns"].mean() / df["strategy_returns"].std() * np.sqrt(252 * 390)
    logger.info("Backtest for %s: total_return=%.4f sharpe=%.4f", symbol, total_return, sharpe)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--config", default="config/pipeline.json")
    args = parser.parse_args()
    backtest(args.symbol, args.config)
