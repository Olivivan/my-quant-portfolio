"""Generate a simple ML-informed price forecast from the last signal to a future date.

The model is a direction classifier.  The forecast takes the most recent predicted
direction and probability for each symbol and applies them as a daily drift scaled
by the symbol's historical daily volatility.  The result is one possible future
price trajectory, not a guarantee.
"""
from __future__ import annotations

import numpy as np
import pandas as pd


def _trading_days(start: pd.Timestamp, end: pd.Timestamp) -> pd.DatetimeIndex:
    """Return business-day timestamps between start and end (inclusive)."""
    return pd.bdate_range(start=start, end=end)


def generate_forecast(
    symbol: str,
    last_price: float,
    last_date: pd.Timestamp,
    end_date: pd.Timestamp,
    signals: pd.DataFrame,
    prices: pd.DataFrame,
) -> pd.DataFrame:
    """Produce a daily price forecast from last_date to end_date.

    Parameters
    ----------
    symbol:
        Ticker symbol.
    last_price:
        Last known close price.
    last_date:
        Date of the last known price/prediction.
    end_date:
        Forecast horizon.
    signals:
        DataFrame with columns [time, symbol, prediction, probability].
    prices:
        DataFrame with columns [time, symbol, close].

    Returns
    -------
    DataFrame with columns [time, symbol, close].
    """
    sig = signals[signals["symbol"] == symbol].sort_values("time")
    if sig.empty:
        return pd.DataFrame(columns=["time", "symbol", "close"])

    last_signal = sig.iloc[-1]
    direction = int(last_signal["prediction"])
    probability = float(last_signal["probability"])

    # Historical daily log-returns for volatility scaling.
    hist = prices[
        (prices["symbol"] == symbol) & (prices["time"] <= last_date)
    ].sort_values("time")
    if len(hist) >= 2:
        log_returns = np.log(hist["close"] / hist["close"].shift(1)).dropna()
        daily_vol = float(log_returns.std()) if len(log_returns) > 1 else 0.01
    else:
        daily_vol = 0.01

    # Daily drift = predicted direction * confidence * historical volatility.
    # A probability near 0.5 implies low confidence -> small drift; near 1 -> large.
    drift = direction * (2 * probability - 1) * daily_vol

    days = _trading_days(last_date + pd.Timedelta(days=1), end_date)
    if len(days) == 0:
        return pd.DataFrame(columns=["time", "symbol", "close"])

    forecast_prices = [last_price]
    for _ in days[1:]:
        forecast_prices.append(forecast_prices[-1] * (1 + drift))

    df = pd.DataFrame({
        "time": days,
        "symbol": symbol,
        "close": forecast_prices,
    })
    return df
