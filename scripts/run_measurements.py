#!/usr/bin/env python3
import argparse
import csv
import os
import re
import subprocess
from datetime import datetime

TIME_RE = re.compile(r"^Time:\s*([0-9.eE+-]+)\s*seconds")
GFLOPS_RE = re.compile(r"^GFlop/s:\s*([0-9.eE+-]+)")
JOULES_RE = re.compile(r"^Joules:\s*([0-9.eE+-]+)")
WATTS_RE = re.compile(r"^Watts:\s*([0-9.eE+-]+)")


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


def build_input_sequence(option, n, block_size=None, threads=None):
    parts = [str(option), str(n)]
    if option == 2:
        parts.append(str(block_size))
    if option == 3:
        parts.append(str(threads))
    parts.append("5")
    return "\n".join(parts) + "\n"


def run_case(bin_path, option, n, block_size, threads, run_id, timeout_s):
    stdin_data = build_input_sequence(option, n, block_size, threads)
    try:
        proc = subprocess.run(
            [bin_path],
            input=stdin_data,
            text=True,
            capture_output=True,
            timeout=timeout_s,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        return {
            "exit_code": -1,
            "error": f"timeout after {timeout_s}s",
            "stdout": exc.stdout or "",
            "stderr": exc.stderr or "",
        }

    return {
        "exit_code": proc.returncode,
        "error": "" if proc.returncode == 0 else (proc.stderr.strip() or "non-zero exit"),
        "stdout": proc.stdout,
        "stderr": proc.stderr,
    }


def parse_int_list(value):
    items = []
    for part in value.split(","):
        part = part.strip()
        if not part:
            continue
        items.append(int(part))
    return items


def load_existing_rows(path, fieldnames):
    if not os.path.exists(path) or os.path.getsize(path) == 0:
        return {}
    rows = {}
    with open(path, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
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


def write_rows(path, fieldnames, rows_by_key):
    tmp_path = f"{path}.tmp"
    implementation_order = {
        "sequential": 1,
        "block": 2,
        "openmp": 3,
        "sycl": 4,
    }
    def sort_key(row):
        option = int(row.get("option", "0") or 0)
        impl = row.get("implementation", "")
        impl_order = implementation_order.get(impl, 99)
        n = int(row.get("n", "0") or 0)
        block_size = int(row.get("block_size", "0") or 0)
        threads = int(row.get("threads", "0") or 0)
        run = int(row.get("run", "0") or 0)
        return (option, impl_order, n, block_size, threads, run)
    with open(tmp_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for row in sorted(rows_by_key.values(), key=sort_key):
            writer.writerow(row)
    os.replace(tmp_path, path)


def normalize_row(row):
    return {k: ("" if v is None else str(v)) for k, v in row.items()}


def main():
    parser = argparse.ArgumentParser(description="Run LU benchmarks and export CSV.")
    parser.add_argument("--bin", default="bin/Assignment2", help="Path to executable")
    parser.add_argument("--out", default="measurements.csv", help="CSV output path")
    parser.add_argument("--sizes", default="1024,2048,3072,4096,5120,6144,7168,8192",
                        help="Comma-separated n sizes")
    parser.add_argument("--block-sizes", default="32,64,128",
                        help="Comma-separated block sizes for option 2")
    parser.add_argument("--threads", default="1,2,4,8,16",
                        help="Comma-separated thread counts for option 3")
    parser.add_argument("--runs", type=int, default=3, help="Repetitions per config")
    parser.add_argument("--options", default="1,2,3,4",
                        help="Menu options to run, comma-separated (1-4)")
    parser.add_argument("--timeout", type=int, default=600, help="Timeout per run (seconds)")
    args = parser.parse_args()

    bin_path = args.bin
    if not os.path.isfile(bin_path):
        raise SystemExit(f"Executable not found: {bin_path}")

    sizes = parse_int_list(args.sizes)
    block_sizes = parse_int_list(args.block_sizes)
    threads_list = parse_int_list(args.threads)
    options = parse_int_list(args.options)

    out_path = args.out
    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    run_counter = 0
    rows_written = 0

    fieldnames = [
        "timestamp",
        "option",
        "implementation",
        "n",
        "block_size",
        "threads",
        "run",
        "time_s",
        "gflops",
        "joules",
        "watts",
        "exit_code",
        "error",
    ]

    rows_by_key = load_existing_rows(out_path, fieldnames)

    for option in options:
        for n in sizes:
            if option == 1:
                for r in range(args.runs):
                    key = (
                        str(option),
                        "sequential",
                        str(n),
                        "",
                        "",
                        str(r + 1),
                    )
                    if key in rows_by_key:
                        print(f"Skip existing: option=1 n={n} run={r + 1}")
                        continue
                    run_counter += 1
                    print(f"Run {run_counter}: option=1 n={n}")
                    result = run_case(bin_path, option, n, None, None, run_counter, args.timeout)
                    time_s, gflops, joules, watts = parse_metrics(result["stdout"])
                    row = {
                        "timestamp": datetime.now().isoformat(timespec="seconds"),
                        "option": option,
                        "implementation": "sequential",
                        "n": n,
                        "block_size": "",
                        "threads": "",
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
            elif option == 2:
                for block_size in block_sizes:
                    for r in range(args.runs):
                        key = (
                            str(option),
                            "block",
                            str(n),
                            str(block_size),
                            "",
                            str(r + 1),
                        )
                        if key in rows_by_key:
                            print(
                                f"Skip existing: option=2 n={n} block_size={block_size} run={r + 1}"
                            )
                            continue
                        run_counter += 1
                        print(f"Run {run_counter}: option=2 n={n} block_size={block_size}")
                        result = run_case(bin_path, option, n, block_size, None, run_counter, args.timeout)
                        time_s, gflops, joules, watts = parse_metrics(result["stdout"])
                        row = {
                            "timestamp": datetime.now().isoformat(timespec="seconds"),
                            "option": option,
                            "implementation": "block",
                            "n": n,
                            "block_size": block_size,
                            "threads": "",
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
            elif option == 3:
                for threads in threads_list:
                    for r in range(args.runs):
                        key = (
                            str(option),
                            "openmp",
                            str(n),
                            "",
                            str(threads),
                            str(r + 1),
                        )
                        if key in rows_by_key:
                            print(f"Skip existing: option=3 n={n} threads={threads} run={r + 1}")
                            continue
                        run_counter += 1
                        print(f"Run {run_counter}: option=3 n={n} threads={threads}")
                        result = run_case(bin_path, option, n, None, threads, run_counter, args.timeout)
                        time_s, gflops, joules, watts = parse_metrics(result["stdout"])
                        row = {
                            "timestamp": datetime.now().isoformat(timespec="seconds"),
                            "option": option,
                            "implementation": "openmp",
                            "n": n,
                            "block_size": "",
                            "threads": threads,
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
            elif option == 4:
                for r in range(args.runs):
                    key = (
                        str(option),
                        "sycl",
                        str(n),
                        "",
                        "",
                        str(r + 1),
                    )
                    if key in rows_by_key:
                        print(f"Skip existing: option=4 n={n} run={r + 1}")
                        continue
                    run_counter += 1
                    print(f"Run {run_counter}: option=4 n={n}")
                    result = run_case(bin_path, option, n, None, None, run_counter, args.timeout)
                    time_s, gflops, joules, watts = parse_metrics(result["stdout"])
                    row = {
                        "timestamp": datetime.now().isoformat(timespec="seconds"),
                        "option": option,
                        "implementation": "sycl",
                        "n": n,
                        "block_size": "",
                        "threads": "",
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
            else:
                raise SystemExit(f"Unknown option: {option}")

    print(f"Wrote {rows_written} rows to {out_path}")


if __name__ == "__main__":
    main()
