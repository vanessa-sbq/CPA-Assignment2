#!/usr/bin/env python3
import argparse
import csv
import os
from collections import defaultdict


# Parse float values; return None for empty or invalid entries.
def parse_float(value):
    if value is None:
        return None
    value = str(value).strip()
    if not value:
        return None
    try:
        return float(value)
    except ValueError:
        return None


# Parse int values; return None for empty or invalid entries.
def parse_int(value):
    if value is None:
        return None
    value = str(value).strip()
    if not value:
        return None
    try:
        return int(value)
    except ValueError:
        return None


def load_rows(path):
    rows = []
    with open(path, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({
                **row,
                "option": parse_int(row.get("option")),
                "n": parse_int(row.get("n")),
                "block_size": parse_int(row.get("block_size")),
                "threads": parse_int(row.get("threads")),
                "time_s_avg": parse_float(row.get("time_s_avg")),
                "gflops_avg": parse_float(row.get("gflops_avg")),
                "joules_avg": parse_float(row.get("joules_avg")),
                "watts_avg": parse_float(row.get("watts_avg")),
                "speedup": parse_float(row.get("speedup")),
                "efficiency": parse_float(row.get("efficiency")),
            })
    return rows


def pick_row_for_impl(rows, impl, n, args):
    cand = [r for r in rows if r.get("implementation") == impl and r.get("n") == n]
    if not cand:
        return None

    if impl == "block":
        if args.block_size is not None:
            cand = [r for r in cand if r.get("block_size") == args.block_size]
        if not cand:
            return None
        return min(cand, key=lambda r: (r.get("time_s_avg") is None, r.get("time_s_avg") or 0.0))

    if impl == "openmp":
        if args.threads is not None:
            cand = [r for r in cand if r.get("threads") == args.threads]
            if not cand:
                return None
            return min(cand, key=lambda r: (r.get("time_s_avg") is None, r.get("time_s_avg") or 0.0))

        # Prefer the maximum thread count for a consistent comparison.
        cand_with_threads = [r for r in cand if r.get("threads") is not None]
        if cand_with_threads:
            max_threads = max(r.get("threads") for r in cand_with_threads)
            cand = [r for r in cand_with_threads if r.get("threads") == max_threads]
        return min(cand, key=lambda r: (r.get("time_s_avg") is None, r.get("time_s_avg") or 0.0))

    return min(cand, key=lambda r: (r.get("time_s_avg") is None, r.get("time_s_avg") or 0.0))


def plot_lines(x_vals, series, xlabel, ylabel, title, out_path):
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(8, 5))
    for label, y_vals in series.items():
        ax.plot(x_vals, y_vals, marker="o", label=label)

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid(True, linestyle="--", alpha=0.4)
    if len(series) > 1:
        ax.legend()
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)


def plot_grouped_lines(groups, xlabel, ylabel, title, out_path):
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(8, 5))
    for label, points in groups.items():
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        ax.plot(xs, ys, marker="o", label=label)

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid(True, linestyle="--", alpha=0.4)
    if len(groups) > 1:
        ax.legend()
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description="Plot measurement graphics from the averaged CSV.")
    parser.add_argument("--in", dest="in_path", default="measurements_avg.csv", help="Input CSV path")
    parser.add_argument("--out-dir", default="plots", help="Output directory for images")
    parser.add_argument("--block-size", type=int, default=None, help="Block size to plot for option 2")
    parser.add_argument("--threads", type=int, default=None, help="Thread count to plot for option 3")
    args = parser.parse_args()

    if not os.path.exists(args.in_path):
        raise SystemExit(f"Input CSV not found: {args.in_path}")

    rows = load_rows(args.in_path)
    if not rows:
        raise SystemExit("No rows found in input CSV.")

    os.makedirs(args.out_dir, exist_ok=True)

    implementations = sorted({r.get("implementation") for r in rows if r.get("implementation")})
    sizes = sorted({r.get("n") for r in rows if r.get("n") is not None})

    # Time and GFLOPS vs n (one line per implementation).
    time_series = {}
    gflops_series = {}
    for impl in implementations:
        time_vals = []
        gflops_vals = []
        for n in sizes:
            row = pick_row_for_impl(rows, impl, n, args)
            time_vals.append(row.get("time_s_avg") if row else None)
            gflops_vals.append(row.get("gflops_avg") if row else None)
        # Skip if all values are missing.
        if any(v is not None for v in time_vals):
            time_series[impl] = time_vals
        if any(v is not None for v in gflops_vals):
            gflops_series[impl] = gflops_vals

    if time_series:
        plot_lines(
            sizes,
            time_series,
            "n",
            "time (s)",
            "Average time vs n",
            os.path.join(args.out_dir, "time_vs_n.png"),
        )

    if gflops_series:
        plot_lines(
            sizes,
            gflops_series,
            "n",
            "GFLOP/s",
            "Average GFLOP/s vs n",
            os.path.join(args.out_dir, "gflops_vs_n.png"),
        )

    # OpenMP scaling: speedup and efficiency vs threads, grouped by n.
    openmp_rows = [r for r in rows if r.get("implementation") == "openmp" and r.get("threads") is not None]
    if openmp_rows:
        speedup_groups = defaultdict(list)
        efficiency_groups = defaultdict(list)
        for r in sorted(openmp_rows, key=lambda r: (r.get("n"), r.get("threads"))):
            n = r.get("n")
            threads = r.get("threads")
            if r.get("speedup") is not None:
                speedup_groups[f"n={n}"].append((threads, r.get("speedup")))
            if r.get("efficiency") is not None:
                efficiency_groups[f"n={n}"].append((threads, r.get("efficiency")))

        if speedup_groups:
            plot_grouped_lines(
                speedup_groups,
                "threads",
                "speedup",
                "OpenMP speedup vs threads",
                os.path.join(args.out_dir, "openmp_speedup_vs_threads.png"),
            )

        if efficiency_groups:
            plot_grouped_lines(
                efficiency_groups,
                "threads",
                "efficiency",
                "OpenMP efficiency vs threads",
                os.path.join(args.out_dir, "openmp_efficiency_vs_threads.png"),
            )

    # Block size sensitivity: time vs block size, grouped by n.
    block_rows = [r for r in rows if r.get("implementation") == "block" and r.get("block_size") is not None]
    if block_rows:
        block_groups = defaultdict(list)
        for r in sorted(block_rows, key=lambda r: (r.get("n"), r.get("block_size"))):
            n = r.get("n")
            block_size = r.get("block_size")
            if r.get("time_s_avg") is not None:
                block_groups[f"n={n}"].append((block_size, r.get("time_s_avg")))

        if block_groups:
            plot_grouped_lines(
                block_groups,
                "block size",
                "time (s)",
                "Block size sensitivity",
                os.path.join(args.out_dir, "block_time_vs_block_size.png"),
            )

    print(f"Plots written to {args.out_dir}")


if __name__ == "__main__":
    main()
