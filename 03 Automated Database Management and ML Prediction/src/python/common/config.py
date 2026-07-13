import json
from pathlib import Path
from typing import Any, Dict


class Config:
    def __init__(self, path: str = "config/pipeline.json"):
        self._path = Path(path)
        with open(self._path) as f:
            self._data: Dict[str, Any] = json.load(f)

    @property
    def raw(self) -> Dict[str, Any]:
        return self._data

    def db_uri(self) -> str:
        d = self._data["database"]
        return (
            f"postgresql://{d['user']}:{d['password']}"
            f"@{d['host']}:{d['port']}/{d['dbname']}"
        )

    @property
    def symbols(self):
        return self._data["ingestion"]["symbols"]

    @property
    def feature_cfg(self):
        return self._data["features"]

    @property
    def ml_cfg(self):
        return self._data["ml"]

    @property
    def inference_cfg(self):
        return self._data["inference"]
