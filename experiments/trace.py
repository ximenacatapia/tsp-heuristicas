import csv
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    if len(sys.argv) < 2:
        print("Usage: plot_trace.py <trace.csv> [output.png]")
        sys.exit(1)

    path = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else path.replace(".csv", ".png")

    batch, current, best = [], [], []
    with open(path) as f:
        for row in csv.DictReader(f):
            batch.append(int(row["batch"]))
            current.append(float(row["current_cost"]))
            best.append(float(row["best_cost"]))

    if not batch:
        print("No data in", path)
        sys.exit(1)

    fig, ax = plt.subplots(figsize=(12, 6))

    # The saw-tooth: the accepted solution's cost at each batch. It rises when a
    # worse solution is accepted (within the threshold) and falls when a better
    # one is found -- the shape that shows the search exploring then settling.
    ax.plot(batch, current, linewidth=0.6, label="current (accepted)")
    # The best-so-far never rises.
    ax.plot(batch, best, linewidth=1.5, label="best so far")

    ax.set_xlabel("batch")
    ax.set_ylabel("cost")
    ax.set_title("Cost trace over the run")
    ax.legend()

    # The early batches can be enormous (non-feasible, huge penalties), which
    # squashes the interesting part. A log scale on y keeps both visible.
    if max(current) > 10 * max(best[len(best) // 2:] or [1]):
        ax.set_yscale("log")
        ax.set_ylabel("cost (log scale)")

    fig.tight_layout()
    fig.savefig(out, dpi=120)
    print("Saved trace plot to", out)
    print(f"batches: {len(batch)}   final best: {best[-1]:.6f}")


if __name__ == "__main__":
    main()
