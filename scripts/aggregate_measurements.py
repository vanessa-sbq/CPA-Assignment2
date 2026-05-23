#!/usr/bin/env python3
import argparse
import csv
import os


# Helper to parse a float value, treating empty strings and None as None, and ignoring invalid values
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


# Main entry point: read the input CSV, group by parameters, average the results, and write the output CSV.
def main():
    parser = argparse.ArgumentParser(description="Aggregate measurement CSV by averaging runs.")
    parser.add_argument("--in", dest="in_path", default="measurements.csv", help="Input CSV path")
    parser.add_argument("--out", dest="out_path", default="measurements_avg.csv", help="Output CSV path")
    args = parser.parse_args()

    if not os.path.exists(args.in_path):
        raise SystemExit(f"Input CSV not found: {args.in_path}")

    groups = {}

    # Read the input CSV and group by (option, implementation, n, block_size, threads), summing time_s, gflops, joules, watts, and counting runs.
    with open(args.in_path, "r", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row.get("exit_code", "0").strip() not in ("", "0"):
                continue
            key = (
                row.get("option", ""),
                row.get("implementation", ""),
                row.get("n", ""),
                row.get("block_size", ""),
                row.get("threads", ""),
            )
            time_s = parse_float(row.get("time_s"))
            gflops = parse_float(row.get("gflops"))
            joules = parse_float(row.get("joules"))
            watts = parse_float(row.get("watts"))
            if key not in groups:
                groups[key] = {"count": 0, "time_s": 0.0, "gflops": 0.0, "joules": 0.0, "watts": 0.0,}
            grp = groups[key]
            if time_s is not None:
                grp["time_s"] += time_s
            if gflops is not None:
                grp["gflops"] += gflops
            if joules is not None:
                grp["joules"] += joules
            if watts is not None:
                grp["watts"] += watts
            grp["count"] += 1

    out_rows = []
    for (option, implementation, n, block_size, threads), grp in groups.items():
        count = grp["count"] or 1
        out_rows.append({
            "option": option,
            "implementation": implementation,
            "n": n,
            "block_size": block_size,
            "threads": threads,
            "runs": grp["count"],
            "time_s_avg": grp["time_s"] / count,
            "gflops_avg": grp["gflops"] / count,
            "joules_avg": grp["joules"] / count,
            "watts_avg": grp["watts"] / count,
        })

    os.makedirs(os.path.dirname(args.out_path) or ".", exist_ok=True)
    with open(args.out_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["option", "implementation", "n", "block_size", "threads", "runs", "time_s_avg", "gflops_avg", "joules_avg", "watts_avg"],)
        writer.writeheader()
        writer.writerows(out_rows)

    print(f"Wrote {len(out_rows)} rows to {args.out_path}")


if __name__ == "__main__":
    main()
