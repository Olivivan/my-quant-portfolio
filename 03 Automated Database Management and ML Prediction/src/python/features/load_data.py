import logging
from typing import List, Optional

import pandas as pd
import psycopg2
from common.config import Config

logger = logging.getLogger(__name__)


class TimescaleLoader:
    def __init__(self, config: Config):
        self.config = config

    def load_bars(
        self,
        symbol: str,
        start: Optional[str] = None,
        end: Optional[str] = None,
    ) -> pd.DataFrame:
        d = self.config.raw["database"]
        conn = psycopg2.connect(
            host=d["host"],
            port=d["port"],
            dbname=d["dbname"],
            user=d["user"],
            password=d["password"],
        )
        query = "SELECT time, open, high, low, close, volume, vwap, trades FROM bars_1m WHERE symbol = %s"
        params: List[str] = [symbol]
        if start:
            query += " AND time >= %s"
            params.append(start)
        if end:
            query += " AND time <= %s"
            params.append(end)
        query += " ORDER BY time"

        df = pd.read_sql(query, conn, params=params, parse_dates=["time"])
        conn.close()
        df.set_index("time", inplace=True)
        logger.info("Loaded %d bars for %s", len(df), symbol)
        return df

    def load_all_symbols(self) -> pd.DataFrame:
        frames = []
        for sym in self.config.symbols:
            df = self.load_bars(sym)
            df["symbol"] = sym
            frames.append(df)
        return pd.concat(frames)
