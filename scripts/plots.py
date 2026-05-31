#!/usr/bin/env python3
import argparse
import os

import matplotlib.pyplot as plt
import pandas as pd

OUTPUT_DIR = "plots"

IMPL_LABELS = {
    "sequential": "Sequential",
    "block":      "Sequential Block",
    "openmp":     "OpenMP",
    "sycl_dumb":  "SYCL Dumb",
    "sycl_basic": "SYCL Basic",
    "sycl_block": "SYCL Block",
}

SYCL_IMPLS = ["sycl_dumb", "sycl_basic", "sycl_block"]


def load_data(csv_path):
    df = pd.read_csv(csv_path)
    for col in ["n", "block_size", "threads", "time_s_avg", "gflops_avg",
                "joules_avg", "watts_avg", "speedup", "efficiency"]:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")
    return df


def save_fig(fig, name):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    path = os.path.join(OUTPUT_DIR, f"{name}.png")
    fig.savefig(path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved {path}")


def add_speedup_seq(df):
    """Add speedup_seq = sequential_time/impl_time for all rows (including block/sycl)."""
    baseline = (
        df[df["implementation"] == "sequential"]
        .dropna(subset=["time_s_avg"])
        .set_index("n")["time_s_avg"]
        .to_dict()
    )
    df = df.copy()

    def _sp(row):
        t1 = baseline.get(row["n"])
        tp = row["time_s_avg"]
        if t1 and tp and tp > 0:
            return t1 / tp
        return None

    df["speedup_seq"] = df.apply(_sp, axis=1)
    return df


def pick_best_block_size(df, impl):
    """Return the block_size with the lowest total time_s_avg across all n."""
    sub = df[(df["implementation"] == impl)].dropna(subset=["block_size", "time_s_avg"])
    if sub.empty:
        return None
    return sub.groupby("block_size")["time_s_avg"].sum().idxmin()


def pick_best_threads(df, impl="openmp"):
    """Return the thread count with the lowest total time_s_avg across all n."""
    sub = df[(df["implementation"] == impl)].dropna(subset=["threads", "time_s_avg"])
    if sub.empty:
        return None
    return sub.groupby("threads")["time_s_avg"].sum().idxmin()


def series_block(df, impl, block_size, metric):
    return (
        df[(df["implementation"] == impl) & (df["block_size"] == block_size)]
        .dropna(subset=[metric])
        .sort_values("n")
    )


def series_threads(df, impl, threads, metric):
    return (
        df[(df["implementation"] == impl) & (df["threads"] == threads)]
        .dropna(subset=[metric])
        .sort_values("n")
    )


def series_all(df, impl, metric):
    return (
        df[df["implementation"] == impl]
        .dropna(subset=[metric])
        .sort_values("n")
    )


### Plot 1: Runtime 
# Series: Sequential | Block (all block sizes) | OpenMP (all threads)* | SYCL *(all 3)

def plot_runtime(df):
    fig, ax = plt.subplots(figsize=(10, 6))

    # Sequential
    s = series_all(df, "sequential", "time_s_avg")
    if not s.empty:
        ax.plot(s["n"], s["time_s_avg"], "o-", label="Sequential", markersize=5)

    # Block - every block size
    for bs in sorted(df[df["implementation"] == "block"]["block_size"].dropna().unique()):
        s = series_block(df, "block", bs, "time_s_avg")
        if not s.empty:
            ax.plot(s["n"], s["time_s_avg"], "s--", label=f"Block (bs={int(bs)})", markersize=5)

    # OpenMP - every thread count (* = best block size, but bs is fixed/absent, so just annotate threads)
    for t in sorted(df[df["implementation"] == "openmp"]["threads"].dropna().unique()):
        s = series_threads(df, "openmp", t, "time_s_avg")
        if not s.empty:
            ax.plot(s["n"], s["time_s_avg"], "^-", label=f"OpenMP ({int(t)}t)", markersize=5)

    # SYCL - best block size per implementation
    for impl, marker in zip(SYCL_IMPLS, ["D-", "v-", "*-"]):
        bs = pick_best_block_size(df, impl)
        if bs is None:
            continue
        s = series_block(df, impl, bs, "time_s_avg")
        if not s.empty:
            ax.plot(s["n"], s["time_s_avg"], marker, label=f"{IMPL_LABELS[impl]} (bs={int(bs)}*)", markersize=5)

    ax.set_xlabel("Matrix size n")
    ax.set_ylabel("Time (s)")
    ax.set_title("Runtime - All Implementations")
    ax.legend(loc="upper left", fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.5)
    save_fig(fig, "1_runtime")


### Plot 2: Speedup (baseline: Sequential)
# Series: Block (all block sizes) | OpenMP (all threads)* | SYCL *(all 3)

def plot_speedup(df):
    fig, ax = plt.subplots(figsize=(10, 6))

    # Block - every block size
    for bs in sorted(df[df["implementation"] == "block"]["block_size"].dropna().unique()):
        s = series_block(df, "block", bs, "speedup_seq")
        if not s.empty:
            ax.plot(s["n"], s["speedup_seq"], "s--", label=f"Block (bs={int(bs)})", markersize=5)

    # OpenMP - every thread count
    for t in sorted(df[df["implementation"] == "openmp"]["threads"].dropna().unique()):
        s = series_threads(df, "openmp", t, "speedup_seq")
        if not s.empty:
            ax.plot(s["n"], s["speedup_seq"], "^-", label=f"OpenMP ({int(t)}t)", markersize=5)

    # SYCL - best block size
    for impl, marker in zip(SYCL_IMPLS, ["D-", "v-", "*-"]):
        bs = pick_best_block_size(df, impl)
        if bs is None:
            continue
        s = series_block(df, impl, bs, "speedup_seq")
        if not s.empty:
            ax.plot(s["n"], s["speedup_seq"], marker, label=f"{IMPL_LABELS[impl]} (bs={int(bs)}*)", markersize=5)

    ax.axhline(1.0, color="black", linestyle=":", linewidth=1, label="Baseline (Sequential=1)")
    ax.set_xlabel("Matrix size n")
    ax.set_ylabel("Speedup vs Sequential")
    ax.set_title("Speedup - All Implementations (baseline: Sequential)")
    ax.legend(loc="best", fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.5)
    save_fig(fig, "2_speedup")


### Helper for plots 3a/3b 
# Series: Sequential | Block* | OpenMP (best threads)* | SYCL *(all 3)

def _plot_single_line_per_impl(df, ax, metric, ylabel):
    # Sequential
    s = series_all(df, "sequential", metric)
    if not s.empty:
        ax.plot(s["n"], s[metric], "o-", label="Sequential", markersize=5)

    # Block - best block size
    bs = pick_best_block_size(df, "block")
    if bs is not None:
        s = series_block(df, "block", bs, metric)
        if not s.empty:
            ax.plot(s["n"], s[metric], "s-", label=f"Block (bs={int(bs)}*)", markersize=5)

    # OpenMP - best thread count
    best_t = pick_best_threads(df, "openmp")
    if best_t is not None:
        s = series_threads(df, "openmp", best_t, metric)
        if not s.empty:
            ax.plot(s["n"], s[metric], "^-", label=f"OpenMP ({int(best_t)}t*)", markersize=5)

    # SYCL - best block size
    for impl, marker in zip(SYCL_IMPLS, ["D-", "v-", "*-"]):
        bs = pick_best_block_size(df, impl)
        if bs is None:
            continue
        s = series_block(df, impl, bs, metric)
        if not s.empty:
            ax.plot(s["n"], s[metric], marker, label=f"{IMPL_LABELS[impl]} (bs={int(bs)}*)", markersize=5)

    ax.set_xlabel("Matrix size n")
    ax.set_ylabel(ylabel)
    ax.legend(loc="upper left", fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.5)


### Plot 3a: Wattage 

def plot_wattage(df):
    fig, ax = plt.subplots(figsize=(10, 6))
    _plot_single_line_per_impl(df, ax, "watts_avg", "Power (W)")
    ax.set_title("Average Power Draw - All Implementations (best config)")
    save_fig(fig, "3a_wattage")


### Plot 3b: Total Energy Spent

def plot_energy(df):
    fig, ax = plt.subplots(figsize=(10, 6))
    _plot_single_line_per_impl(df, ax, "joules_avg", "Energy (J)")
    ax.set_title("Total Energy Spent - All Implementations (best config)")
    save_fig(fig, "3b_energy")


### Plot 4: OpenMP Efficiency (all threads)
# efficiency = speedup / threads, for all thread counts

def plot_openmp_efficiency(df):
    fig, ax = plt.subplots(figsize=(10, 6))

    omp = df[df["implementation"] == "openmp"].dropna(subset=["speedup_seq", "threads"])
    for t in sorted(omp["threads"].dropna().unique()):
        s = omp[omp["threads"] == t].sort_values("n").copy()
        if s.empty:
            continue
        s["eff"] = s["speedup_seq"] / t
        ax.plot(s["n"], s["eff"], "o-", label=f"OpenMP ({int(t)}t)", markersize=5)

    ax.axhline(1.0, color="black", linestyle=":", linewidth=1, label="Ideal (efficiency=1)")
    ax.set_xlabel("Matrix size n")
    ax.set_ylabel("Efficiency (speedup / threads)")
    ax.set_title("Parallel Efficiency - OpenMP (all thread counts)")
    ax.legend(fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.5)
    save_fig(fig, "4_openmp_efficiency")


### Plot 5: SYCL Efficiency (GFlop/s, all implementations, all block sizes) 

def plot_sycl_efficiency(df):
    fig, ax = plt.subplots(figsize=(10, 6))

    colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]
    linestyles = ["-", "--", ":"]
    i = 0

    for li, impl in enumerate(SYCL_IMPLS):
        sub = df[df["implementation"] == impl]
        for bs in sorted(sub["block_size"].dropna().unique()):
            s = series_block(df, impl, bs, "gflops_avg")
            if s.empty:
                continue
            ax.plot(s["n"], s["gflops_avg"],
                    linestyles[li],
                    color=colors[i % len(colors)],
                    marker="o", markersize=4,
                    label=f"{IMPL_LABELS[impl]} (bs={int(bs)})")
            i += 1

    ax.set_xlabel("Matrix size n")
    ax.set_ylabel("GFlop/s")
    ax.set_title("SYCL Efficiency (GFlop/s) - All Implementations, All Block Sizes")
    ax.legend(loc="best", fontsize=7, ncol=2)
    ax.grid(True, linestyle="--", alpha=0.5)
    save_fig(fig, "5_sycl_efficiency")


### Plot 6: OpenMP Scalability (speedup vs. threads) 

def plot_openmp_scalability(df):
    fig, ax = plt.subplots(figsize=(10, 6))

    omp = df[df["implementation"] == "openmp"].dropna(subset=["speedup_seq", "threads"])
    colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]

    for j, n in enumerate(sorted(omp["n"].dropna().unique())):
        group = omp[omp["n"] == n].sort_values("threads")
        if group.empty:
            continue
        ax.plot(group["threads"], group["speedup_seq"], "o-",
                color=colors[j % len(colors)],
                label=f"n={int(n)}", markersize=5)

    all_threads = sorted(omp["threads"].dropna().unique().astype(int))
    # Ideal linear speedup line (capped at thread range)
    ax.plot(all_threads, all_threads, "k--", linewidth=1, label="Ideal (linear)", alpha=0.5)
    ax.set_xlabel("Number of Threads")
    ax.set_ylabel("Speedup vs Sequential")
    ax.set_title("OpenMP Scalability - Speedup vs Threads")
    ax.set_xticks(all_threads)
    ax.legend(loc="upper left", ncol=2, fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.5)
    save_fig(fig, "6_openmp_scalability")


### Plot 7: SYCL Scalability (GFlop/s vs. block size) 
# One figure per SYCL implementation, lines per problem size n

def plot_sycl_scalability(df):
    colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]

    for impl in SYCL_IMPLS:
        fig, ax = plt.subplots(figsize=(10, 6))
        sub = df[df["implementation"] == impl].dropna(subset=["block_size", "gflops_avg"])

        for j, n in enumerate(sorted(sub["n"].dropna().unique())):
            group = sub[sub["n"] == n].sort_values("block_size")
            if group.empty:
                continue
            ax.plot(group["block_size"], group["gflops_avg"], "o-",
                    color=colors[j % len(colors)],
                    label=f"n={int(n)}", markersize=5)

        bsizes = sorted(sub["block_size"].dropna().unique().astype(int))
        ax.set_xlabel("Block Size (work-group size)")
        ax.set_ylabel("GFlop/s")
        ax.set_title(f"SYCL Scalability - {IMPL_LABELS[impl]}: GFlop/s vs Block Size")
        ax.set_xticks(bsizes)
        ax.legend(loc="best", ncol=2, fontsize=8)
        ax.grid(True, linestyle="--", alpha=0.5)
        save_fig(fig, f"7_sycl_scalability_{impl}")


### Main 

def main():
    parser = argparse.ArgumentParser(description="Generate plots from aggregated measurements.")
    parser.add_argument("--in", dest="in_path", default="measurements_avg.csv",
                        help="Input CSV (default: measurements_avg.csv)")
    args = parser.parse_args()

    if not os.path.exists(args.in_path):
        raise SystemExit(f"Input file not found: {args.in_path}")

    df = load_data(args.in_path)
    df = add_speedup_seq(df)

    plot_runtime(df)
    plot_speedup(df)
    plot_wattage(df)
    plot_energy(df)
    plot_openmp_efficiency(df)
    plot_sycl_efficiency(df)
    plot_openmp_scalability(df)
    plot_sycl_scalability(df)


if __name__ == "__main__":
    main()
