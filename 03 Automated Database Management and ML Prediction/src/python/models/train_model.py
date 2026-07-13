"""Train a LightGBM direction model with purged CV and export to ONNX."""

import argparse
import json
import logging
import sys
from pathlib import Path

import joblib
import numpy as np
import pandas as pd
from common.config import Config
from features.engineer_features import (
    FEATURE_COLUMNS,
    build_target,
    clean_and_select,
    compute_features,
)
from features.load_data import TimescaleLoader
from lightgbm import LGBMClassifier
from models.cv_utils import PurgedKFold
from onnxmltools import convert_lightgbm
from onnxmltools.convert.common.data_types import FloatTensorType
from sklearn.inspection import permutation_importance
from sklearn.metrics import accuracy_score, f1_score, log_loss

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)
logger = logging.getLogger(__name__)


def cost_adjusted_profit(y_true: np.ndarray, y_pred: np.ndarray, ret: np.ndarray, cost_bps: float) -> float:
    """Profit in bps after transaction cost for binary up/down signals."""
    return np.sum((y_pred * ret - np.abs(np.diff(np.concatenate([[0], y_pred]))) * cost_bps * 0.0001) * 10000)


def train(config_path: str):
    cfg = Config(config_path)
    loader = TimescaleLoader(cfg)
    feature_cfg = cfg.feature_cfg
    ml_cfg = cfg.ml_cfg

    all_data = []
    for sym in cfg.symbols:
        df = loader.load_bars(sym)
        df = compute_features(df, feature_cfg)
        df = build_target(df, feature_cfg["target_horizon_bars"], feature_cfg["target_threshold"])
        df = clean_and_select(df)
        df["symbol"] = sym
        all_data.append(df)

    df = pd.concat(all_data)
    df = df.sort_index()

    X = df[FEATURE_COLUMNS].astype(np.float32)
    # Map {-1,0,1} to {0,1,2} for LightGBM
    y = df["target_direction"].map({-1: 0, 0: 1, 1: 2}).astype(int)

    model = LGBMClassifier(
        n_estimators=500,
        learning_rate=0.05,
        max_depth=6,
        num_leaves=31,
        class_weight="balanced",
        random_state=ml_cfg.get("random_state", 42),
        n_jobs=-1,
        reg_alpha=0.1,
        reg_lambda=0.1,
    )

    cv = PurgedKFold(n_splits=ml_cfg["cv_folds"], embargo_pct=ml_cfg["embargo_pct"])
    fold_metrics = []
    for fold, (train_idx, test_idx) in enumerate(cv.split(X, y)):
        X_train, X_test = X.iloc[train_idx], X.iloc[test_idx]
        y_train, y_test = y.iloc[train_idx], y.iloc[test_idx]
        model.fit(X_train, y_train)
        preds = model.predict(X_test)
        probas = model.predict_proba(X_test)

        metrics = {
            "fold": fold + 1,
            "accuracy": accuracy_score(y_test, preds),
            "f1_macro": f1_score(y_test, preds, average="macro"),
            "log_loss": log_loss(y_test, probas),
        }
        fold_metrics.append(metrics)
        logger.info("Fold %d metrics: %s", fold + 1, metrics)

    # Final fit on all data
    model.fit(X, y)

    # Feature selection by permutation importance
    perm = permutation_importance(model, X.sample(min(5000, len(X)), random_state=42), y.sample(min(5000, len(y)), random_state=42), n_repeats=5, n_jobs=-1)
    importance = pd.Series(perm.importances_mean, index=FEATURE_COLUMNS).sort_values(ascending=False)
    logger.info("Permutation importance:\n%s", importance)

    # Export to ONNX
    onnx_path = Path(ml_cfg["onnx_output_path"])
    onnx_path.parent.mkdir(parents=True, exist_ok=True)
    initial_type = [("float_input", FloatTensorType([None, len(FEATURE_COLUMNS)]))]
    # zipmap=False emits probabilities as a dense [batch, 3] tensor instead
    # of a sequence of maps, which is required by the C++ ONNX engine.
    onnx_model = convert_lightgbm(model, initial_types=initial_type, target_opset=12, zipmap=False)
    with open(onnx_path, "wb") as f:
        f.write(onnx_model.SerializeToString())
    logger.info("Exported ONNX model to %s", onnx_path)

    # Save model registry metadata
    registry = {
        "model_name": "lgbm_direction",
        "version": "1.0.0",
        "onnx_path": str(onnx_path),
        "feature_columns": FEATURE_COLUMNS,
        "cv_metrics": fold_metrics,
        "feature_importance": importance.to_dict(),
        "hyperparameters": model.get_params(),
    }
    with open("models/model_registry.json", "w") as f:
        json.dump(registry, f, indent=2, default=str)
    logger.info("Training complete")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", default="config/pipeline.json")
    args = parser.parse_args()
    train(args.config)
