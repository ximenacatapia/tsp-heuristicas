#!/usr/bin/env python3
"""
Plots the results of a seed sweep (one run per seed).

Usage:
    experiments/plot.py <results.csv> [output.png]

Reads a CSV produced by run.sh / run_parallel.sh (columns:
instance,seed,batch_size,cooling,epsilon,accept_percentage,
initial_temperature,cost,feasible) and draws two panels:
  - the cost of each seed, feasible ones highlighted, best marked
  - a histogram of the feasible costs

This is the sweep plot. For the saw-tooth of a single run, use trace.py.
"""

import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) < 2:
        print("Usage: plot.py <results.csv> [output.png]")
        sys.exit(1)

    path = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else path.replace(".csv", ".png")

    seeds, costs, feasible = [], [], []
    instance = "unknown"
    with open(path) as f:
        for row in csv.DictReader(f):
            instance = row.get("instance", instance)
            seeds.append(int(row["seed"]))
            costs.append(float(row["cost"]))
            feasible.append(row["feasible"] == "1")

    if not costs:
        print("No data in", path)
        sys.exit(1)

    # Only feasible runs matter for the "best" and the histogram: a
    # non-feasible cost is a huge penalty number, not a real tour length.
    feas_costs = [c for c, ok in zip(costs, feasible) if ok]
    best = min(feas_costs) if feas_costs else min(costs)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

    # Left: cost per seed. Feasible and non-feasible in different markers.
    feas_x = [s for s, ok in zip(seeds, feasible) if ok]
    feas_y = [c for c, ok in zip(costs, feasible) if ok]
    inf_x = [s for s, ok in zip(seeds, feasible) if not ok]
    inf_y = [c for c, ok in zip(costs, feasible) if not ok]

    ax1.scatter(feas_x, feas_y, s=10, label="feasible")
    if inf_x:
        ax1.scatter(inf_x, inf_y, s=10, marker="x", label="non-feasible")
    ax1.axhline(best, linestyle="--", linewidth=1, label=f"best = {best:.5f}")
    ax1.set_xlabel("seed")
    ax1.set_ylabel("cost")
    ax1.set_title(f"Cost per seed  ({instance})")
    # Non-feasible costs are astronomically larger; log keeps both visible.
    if inf_y:
        ax1.set_yscale("log")
        ax1.set_ylabel("cost (log scale)")
    ax1.legend()

    # Right: distribution of the feasible costs only.
    if feas_costs:
        ax2.hist(feas_costs, bins=30)
        ax2.axvline(best, linestyle="--", linewidth=1, color="C1")
    ax2.set_xlabel("cost")
    ax2.set_ylabel("count")
    ax2.set_title("Feasible cost distribution")

    n = len(costs)
    n_feas = len(feas_costs)
    avg = sum(feas_costs) / n_feas if n_feas else float("nan")
    fig.suptitle(f"{n} runs | feasible {n_feas}/{n} | "
                 f"best {best:.5f} | avg {avg:.5f}")

    fig.tight_layout()
    fig.savefig(out, dpi=120)
    print("Saved plot to", out)
    print(f"runs: {n}   feasible: {n_feas}/{n}   best: {best:.6f}")


if __name__ == "__main__":
    main()