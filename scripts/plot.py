#!/usr/bin/env python3
import argparse
import csv
import os
from typing import List, Tuple

import matplotlib.pyplot as plt


def read_csv(path: str) -> Tuple[List[int], List[float], List[float]]:
    sizes: List[int] = []
    map_ms: List[float] = []
    umap_ms: List[float] = []
    with open(path, "r", newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            sizes.append(int(row["size"]))
            map_ms.append(float(row["map_ms"]))
            umap_ms.append(float(row["unordered_map_ms"]))
    return sizes, map_ms, umap_ms


def plot_single(x: List[int], y: List[float], title: str, ylabel: str, out_path: str) -> None:
    fig, ax = plt.subplots(figsize=(7.5, 4.5), dpi=140)
    ax.plot(x, y, marker="o", linewidth=2)
    ax.set_title(title)
    ax.set_xlabel("Dataset size (N)")
    ax.set_ylabel(ylabel)
    ax.grid(True, linestyle="--", alpha=0.4)
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)


def plot_compare(x: List[int], map_ms: List[float], umap_ms: List[float], out_path: str) -> None:
    fig, ax = plt.subplots(figsize=(7.5, 4.5), dpi=140)
    ax.plot(x, map_ms, marker="o", linewidth=2, label="map (balanced tree)")
    ax.plot(x, umap_ms, marker="o", linewidth=2, label="unordered_map (hash)")
    ax.set_title("Map vs unordered_map")
    ax.set_xlabel("Dataset size (N)")
    ax.set_ylabel("Time (ms)")
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(description="Plot map vs unordered_map benchmark results.")
    parser.add_argument("--input", default="results/benchmark.csv", help="CSV file produced by the benchmark")
    parser.add_argument("--outdir", default="plots", help="Output directory for plots")
    args = parser.parse_args()

    sizes, map_ms, umap_ms = read_csv(args.input)

    os.makedirs(args.outdir, exist_ok=True)

    plot_single(
        sizes,
        map_ms,
        "map time vs dataset size",
        "Time (ms)",
        os.path.join(args.outdir, "map_time.png"),
    )
    plot_single(
        sizes,
        umap_ms,
        "unordered_map time vs dataset size",
        "Time (ms)",
        os.path.join(args.outdir, "unordered_map_time.png"),
    )
    plot_compare(sizes, map_ms, umap_ms, os.path.join(args.outdir, "comparison.png"))


if __name__ == "__main__":
    main()
