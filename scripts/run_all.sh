#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PYTHON_BIN="${PYTHON_BIN:-$HOME/envs/env312/bin/python}"

cd "$ROOT_DIR"

make
./bin/benchmark "$@"
"$PYTHON_BIN" scripts/plot.py --input results/benchmark.csv --outdir plots
