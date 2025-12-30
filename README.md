# Map vs unordered_map benchmark

This small platform benchmarks `std::map` vs `std::unordered_map` on a workload where
balanced-tree lookups are `O(log N)` and hash lookups are `O(1)` on average.

## What it does

- Generates datasets of shuffled integer keys for multiple sizes.
- Inserts and queries keys in both containers.
- Logs "time of run against size of data" to a CSV and a log file.
- Plots time vs dataset size for each container, plus a comparison plot.

## Build

```bash
make
```

## Run the benchmark

```bash
./bin/benchmark
```

You can customize sizes and trials:

```bash
./bin/benchmark --sizes 1000,5000,20000,80000 --trials 7
```

Outputs:

- `results/benchmark.csv`
- `results/run.log`

## Plot with Python (env312)

```bash
~/envs/env312/bin/python scripts/plot.py --input results/benchmark.csv --outdir plots
```

Generated plots:

- `plots/map_time.png`
- `plots/unordered_map_time.png`
- `plots/comparison.png`

## One-shot script

```bash
scripts/run_all.sh
```

Set a different Python executable if needed:

```bash
PYTHON_BIN=~/envs/env312/bin/python scripts/run_all.sh
```
