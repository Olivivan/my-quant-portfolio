#!/usr/bin/env python3
import argparse
import json
import re
import sys
from pathlib import Path


def parse_metrics(path: Path) -> dict[str, float]:
    metrics: dict[str, float] = {}
    pattern = re.compile(r"^([a-z_]+)=([0-9]+(?:\.[0-9]+)?)$")

    for line in path.read_text(encoding="utf-8").splitlines():
        match = pattern.match(line.strip())
        if not match:
            continue
        key, value = match.groups()
        metrics[key] = float(value)

    return metrics


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate benchmark throughput threshold.")
    parser.add_argument("--input", required=True, help="Path to benchmark output file")
    parser.add_argument("--threshold-config", required=True, help="Path to JSON threshold config")
    args = parser.parse_args()

    input_path = Path(args.input)
    threshold_path = Path(args.threshold_config)

    if not input_path.exists():
        print(f"ERROR: benchmark output not found: {input_path}")
        return 2
    if not threshold_path.exists():
        print(f"ERROR: threshold config not found: {threshold_path}")
        return 2

    metrics = parse_metrics(input_path)
    config = json.loads(threshold_path.read_text(encoding="utf-8"))

    required = ["benchmark_orders", "benchmark_elapsed_ms", "benchmark_orders_per_sec"]
    missing = [k for k in required if k not in metrics]
    if missing:
        print(f"ERROR: missing benchmark metrics: {', '.join(missing)}")
        return 2

    min_ops = float(config["min_orders_per_sec"])
    actual_ops = metrics["benchmark_orders_per_sec"]

    print("Benchmark summary")
    print(f"- orders: {int(metrics['benchmark_orders'])}")
    print(f"- elapsed_ms: {metrics['benchmark_elapsed_ms']:.3f}")
    print(f"- orders_per_sec: {actual_ops:.0f}")
    print(f"- required_min_orders_per_sec: {min_ops:.0f}")

    if actual_ops < min_ops:
        print("ERROR: benchmark throughput below configured minimum")
        return 1

    print("PASS: benchmark throughput threshold satisfied")
    return 0


if __name__ == "__main__":
    sys.exit(main())
