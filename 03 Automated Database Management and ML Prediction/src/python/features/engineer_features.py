import logging
from typing import List

import numpy as np
import pandas as pd

logger = logging.getLogger(__name__)

FEATURE_COLUMNS = [
    "returns",
    "log_returns",
    "realized_vol",
    "sma_5",
    "sma_20",
    "ema_12",
    "rsi_14",
    "macd",
    "macd_signal",
    "bb_position",
    "momentum_10",
    "volume_sma_10",
]


def compute_features(df: pd.DataFrame, cfg: dict) -> pd.DataFrame:
    """Compute technical features aligned with C++ feature engine."""
    df = df.copy()
    df["returns"] = df["close"].pct_change()
    df["log_returns"] = np.log(df["close"] / df["close"].shift(1))

    windows = cfg.get("lookback_windows", [5, 10, 20, 50])
    df["sma_5"] = df["close"].rolling(5).mean()
    df["sma_20"] = df["close"].rolling(20).mean()
    df["volume_sma_10"] = df["volume"].rolling(10).mean()

    # EMA
    df["ema_12"] = df["close"].ewm(span=cfg["macd_fast"], adjust=False).mean()
    ema_slow = df["close"].ewm(span=cfg["macd_slow"], adjust=False).mean()
    df["macd"] = df["ema_12"] - ema_slow
    df["macd_signal"] = df["macd"].ewm(span=cfg["macd_signal"], adjust=False).mean()

    # RSI
    delta = df["close"].diff()
    gain = delta.where(delta > 0, 0.0)
    loss = -delta.where(delta < 0, 0.0)
    avg_gain = gain.ewm(alpha=1 / cfg["rsi_period"], adjust=False).mean()
    avg_loss = loss.ewm(alpha=1 / cfg["rsi_period"], adjust=False).mean()
    rs = avg_gain / avg_loss
    df["rsi_14"] = 100 - (100 / (1 + rs))

    # Bollinger %B
    sma_bb = df["close"].rolling(cfg["bb_period"]).mean()
    std_bb = df["close"].rolling(cfg["bb_period"]).std()
    upper = sma_bb + cfg["bb_std"] * std_bb
    lower = sma_bb - cfg["bb_std"] * std_bb
    df["bb_position"] = (df["close"] - lower) / (upper - lower)

    # Realized vol (annualized intraday)
    df["realized_vol"] = (
        df["log_returns"].rolling(20).std() * np.sqrt(252 * 390)
    )

    df["momentum_10"] = df["close"] / df["close"].shift(10) - 1

    return df


def build_target(df: pd.DataFrame, horizon: int, threshold: float) -> pd.DataFrame:
    fwd = df["close"].shift(-horizon) / df["close"] - 1
    df["target_return"] = fwd
    df["target_direction"] = 0
    df.loc[fwd > threshold, "target_direction"] = 1
    df.loc[fwd < -threshold, "target_direction"] = -1
    return df


def clean_and_select(df: pd.DataFrame) -> pd.DataFrame:
    df = df.replace([np.inf, -np.inf], np.nan)
    df = df.dropna(subset=FEATURE_COLUMNS + ["target_direction"])
    return df
