## Scripts overview

This folder contains two scripts:

- `run_measurements.py` runs the LU binary multiple times and writes raw results to a CSV.
- `aggregate_measurements.py` averages the raw results into a final CSV.

## Prerequisites

- Build the project so the executable exists (default: `bin/Assignment2`).
- If you want energy readings, you need read access to the powercap file. On most systems this requires sudo.

## Run measurements

Default run (all options, all sizes, 3 runs):

```bash
python3 scripts/run_measurements.py --bin bin/Assignment2 --out measurements.csv
```

With sudo for energy readings:

```bash
sudo python3 scripts/run_measurements.py --bin bin/Assignment2 --out measurements.csv
```

Common flags:

- `--options 1,2,3,4` (menu options to run)
- `--sizes 1024,2048,3072` (matrix sizes)
- `--block-sizes 32,64,128` (block LU sizes)
- `--threads 1,2,4,8,16` (OpenMP thread counts)
- `--runs 3` (repetitions per config)

### Replace behavior

The script overwrites existing rows with the same key:

```
option, implementation, n, block_size, threads, run
```

This makes it safe to re-run just one option or size and keep the CSV consistent. The file is rewritten after each run, so partial results are preserved if the script stops.

## Aggregate results

This script averages the runs from the raw CSV and writes a new CSV:

```bash
python3 scripts/aggregate_measurements.py --in measurements.csv --out measurements_avg.csv
```

It ignores rows with a non-zero `exit_code` and overwrites the output file each time.
