#!/usr/bin/env python3
import argparse
import csv
import os
import re
import subprocess
from datetime import datetime

# Regexes to pull out the metrics from the program output
TIME_RE = re.compile(r"^Time:\s*([0-9.eE+-]+)\s*seconds")
GFLOPS_RE = re.compile(r"^GFlop/s:\s*([0-9.eE+-]+)")
JOULES_RE = re.compile(r"^Joules:\s*([0-9.eE+-]+)")
WATTS_RE = re.compile(r"^Watts:\s*([0-9.eE+-]+)")

# Map menu options to implementation names for easier CSV readability.
OPTION_CONFIGS = {
    1: {"implementation": "sequential"},
    2: {"implementation": "block"},
    3: {"implementation": "openmp"},
    4: {"implementation": "sycl"},
}
BLOCK_SIZES = []
THREADS_LIST = []


# Parse the program output lines and pull out the metrics we care about.
def parse_metrics(output):
    time_s = gflops = joules = watts = None
    for line in output.splitlines():
        line = line.strip()
        m = TIME_RE.match(line)
        if m:
            time_s = float(m.group(1))
            continue
        m = GFLOPS_RE.match(line)
        if m:
            gflops = float(m.group(1))
            continue
        m = JOULES_RE.match(line)
        if m:
            joules = float(m.group(1))
            continue
        m = WATTS_RE.match(line)
        if m:
            watts = float(m.group(1))
            continue
    return time_s, gflops, joules, watts


# Feed menu choices in order, ending with "5" to exit.
def build_input_sequence(option, n, block_size=None, threads=None):
    parts = [str(option), str(n)]
    if option == 2:
        parts.append(str(block_size))
    if option == 3:
        parts.append(str(threads))
    parts.append("5")
    return "\n".join(parts) + "\n"


# Run the binary once with a scripted stdin sequence.
def run_case(bin_path, option, n, block_size, threads, run_id):
    # Build the stdin data to feed into the program based on the menu options and parameters.
    stdin_data = build_input_sequence(option, n, block_size, threads)
    proc = subprocess.run([bin_path], input=stdin_data, text=True, capture_output=True, check=False)
    # For normal completion, we return the exit code, any error message (empty if exit code is 0), and the captured stdout/stderr.
    return {
        "exit_code": proc.returncode,
        "error": "" if proc.returncode == 0 else (proc.stderr.strip() or "non-zero exit"),
        "stdout": proc.stdout,
        "stderr": proc.stderr,
    }


# Utility to parse a comma-separated list of integers from a string argument.
def parse_int_list(value):
    items = []
    for part in value.split(","):
        part = part.strip()
        if not part:
            continue
        items.append(int(part))
    return items


# Load existing rows from the CSV if it exists, so we can skip already-completed runs and append new results without losing old ones.
def load_existing_rows(path, fieldnames):
    if not os.path.exists(path) or os.path.getsize(path) == 0:
        return {}
    rows = {}
    with open(path, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            # Key identifies a single "cell" in the spreadsheet.
            key = (
                row.get("option", ""),
                row.get("implementation", ""),
                row.get("n", ""),
                row.get("block_size", ""),
                row.get("threads", ""),
                row.get("run", ""),
            )
            rows[key] = row
    return rows


# Helper to sort rows in a consistent order 
IMPLEMENTATION_ORDER = {"sequential": 1, "block": 2, "openmp": 3, "sycl": 4}
def measurement_sort_key(row):
    option = int(row.get("option", "0") or 0)
    impl = row.get("implementation", "")
    impl_order = IMPLEMENTATION_ORDER.get(impl, 99)
    n = int(row.get("n", "0") or 0)
    block_size = int(row.get("block_size", "0") or 0)
    threads = int(row.get("threads", "0") or 0)
    run = int(row.get("run", "0") or 0)
    return (option, impl_order, n, block_size, threads, run)


# Write a sorted snapshot so the CSV is stable and easy to scan.
def write_rows(path, fieldnames, rows_by_key):
    tmp_path = f"{path}.tmp"
    with open(tmp_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for row in sorted(rows_by_key.values(), key=measurement_sort_key):
            writer.writerow(row)
    os.replace(tmp_path, path)


# Normalize a row dictionary by converting all values to strings and replacing None with empty strings, since CSV wants strings
def normalize_row(row):
    return {k: ("" if v is None else str(v)) for k, v in row.items()}


# Some options have multiple values to iterate over, returns a list of those
def iter_option_values(opt):
    if opt == 2:
        return BLOCK_SIZES
    if opt == 3:
        return THREADS_LIST
    return [None]


# Debug helper to build a consistent status message for each run, showing the parameters being used.
def build_status_message(prefix, option, n, block_size, threads, run_index):
    parts = [f"option={option}", f"n={n}"]
    if block_size is not None:
        parts.append(f"block_size={block_size}")
    if threads is not None:
        parts.append(f"threads={threads}")
    parts.append(f"run={run_index}")
    return f"{prefix}: " + " ".join(parts)


# Main entry point: parse arguments, loop over all combinations of options and parameters, run the binary, collect results, and write to CSV.
def main():
    global BLOCK_SIZES, THREADS_LIST
    parser = argparse.ArgumentParser(description="Run LU benchmarks and export CSV.")
    parser.add_argument("--bin", default="bin/Assignment2", help="Path to executable")
    parser.add_argument("--out", default="measurements.csv", help="CSV output path")
    parser.add_argument("--sizes", default="1024,2048,3072,4096,5120,6144,7168,8192", help="Comma-separated n sizes")
    parser.add_argument("--block-sizes", default="32,64,128", help="Comma-separated block sizes for option 2")
    parser.add_argument("--threads", default="1,2,4,8,16", help="Comma-separated thread counts for option 3")
    parser.add_argument("--runs", type=int, default=3, help="Repetitions per config")
    parser.add_argument("--options", default="1,2,3,4", help="Menu options to run, comma-separated (1-4)")
    args = parser.parse_args()

    bin_path = args.bin
    if not os.path.isfile(bin_path):
        raise SystemExit(f"Executable not found: {bin_path}") 

    # Parse args
    sizes = parse_int_list(args.sizes)
    BLOCK_SIZES = parse_int_list(args.block_sizes)
    THREADS_LIST = parse_int_list(args.threads)
    options = parse_int_list(args.options)

    # Create output directory if needed 
    out_path = args.out
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    
    # Track how many runs we do and how many rows we write
    run_counter = 0
    rows_written = 0
    fieldnames = ["timestamp", "option", "implementation", "n", "block_size", "threads", "run", "time_s", "gflops", "joules", "watts", "exit_code", "error"]

    # Load existing rows so we can skip already-completed runs and append new results without losing old ones.
    rows_by_key = load_existing_rows(out_path, fieldnames)

    # Loop over all combinations of options and parameters, running the binary and collecting results.
    for option in options:
        # Validate the option and get the implementation name for reporting.
        if option not in OPTION_CONFIGS:
            raise SystemExit(f"Unknown option: {option}")

        implementation = OPTION_CONFIGS[option]["implementation"]
        for n in sizes:
            for opt_value in iter_option_values(option):
                block_size = opt_value if option == 2 else None
                threads = opt_value if option == 3 else None
                for r in range(args.runs):
                    key = (
                        str(option),
                        implementation,
                        str(n),
                        "" if block_size is None else str(block_size),
                        "" if threads is None else str(threads),
                        str(r + 1),
                    )

                    # Check if we already have this result from a previous run, and skip if so.
                    if key in rows_by_key:
                        print(build_status_message("Skip existing", option, n, block_size, threads, r + 1))
                        continue

                    run_counter += 1
                    run_label = build_status_message(f"Run {run_counter}", option, n, block_size, threads, r + 1)
                    print(run_label)
                    result = run_case(bin_path, option, n, block_size, threads, run_counter)
                    time_s, gflops, joules, watts = parse_metrics(result["stdout"])
                    row = {
                        "timestamp": datetime.now().isoformat(timespec="seconds"),
                        "option": option,
                        "implementation": implementation,
                        "n": n,
                        "block_size": "" if block_size is None else block_size,
                        "threads": "" if threads is None else threads,
                        "run": r + 1,
                        "time_s": time_s,
                        "gflops": gflops,
                        "joules": joules,
                        "watts": watts,
                        "exit_code": result["exit_code"],
                        "error": result["error"],
                    }
                    rows_by_key[key] = normalize_row(row)
                    write_rows(out_path, fieldnames, rows_by_key)
                    rows_written += 1

    print(f"Wrote {rows_written} rows to {out_path}")


if __name__ == "__main__":
    main()
