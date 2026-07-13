"""Time-series aware cross-validation utilities to prevent look-ahead leakage."""

import logging
from typing import Iterator, Tuple

import numpy as np
import pandas as pd

logger = logging.getLogger(__name__)


class PurgedKFold:
    """K-fold CV with purged observations between train and test to prevent leakage."""

    def __init__(self, n_splits: int = 5, purge_pct: float = 0.02, embargo_pct: float = 0.02):
        self.n_splits = n_splits
        self.purge_pct = purge_pct
        self.embargo_pct = embargo_pct

    def split(
        self, X: pd.DataFrame, y=None, groups=None
    ) -> Iterator[Tuple[np.ndarray, np.ndarray]]:
        n = len(X)
        indices = np.arange(n)
        fold_size = n // self.n_splits

        for i in range(self.n_splits):
            test_start = i * fold_size
            test_end = n if i == self.n_splits - 1 else (i + 1) * fold_size

            purge = max(1, int((test_end - test_start) * self.purge_pct))
            embargo = max(1, int((test_end - test_start) * self.embargo_pct))

            test_idx = indices[test_start:test_end]
            train_idx = np.concatenate([
                indices[: max(0, test_start - purge)],
                indices[min(n, test_end + embargo) :],
            ])
            yield train_idx, test_idx


def walk_forward_split(
    df: pd.DataFrame, n_splits: int = 5
) -> Iterator[Tuple[pd.DataFrame, pd.DataFrame]]:
    n = len(df)
    for i in range(1, n_splits + 1):
        split_point = int(n * i / (n_splits + 1))
        train = df.iloc[:split_point]
        test = df.iloc[split_point : int(n * (i + 1) / (n_splits + 1))]
        yield train, test
