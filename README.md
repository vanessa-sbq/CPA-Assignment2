# CPA Assignment 2 - Shared Memory LU Factorization

This project contains the implementations for LU factorization in five variants:

1. Sequential LU
2. Block-oriented sequential LU
3. Shared-memory OpenMP LU
4. SYCL LU (Basic)
5. SYCL LU (Block)

## How to Run

### 1. Configure

Configure the project with CMake and replace `your-sycl-compiler` with your SYCL compiler (e.g. `acpp` for [AdaptiveCpp](https://github.com/AdaptiveCpp/AdaptiveCpp) or `icpx` for [Intel oneAPI DPC++/C++](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html)):

```bash
cmake -DCMAKE_BUILD_TYPE:STRING=Release --no-warn-unused-cli -S ./src -B ./build -DCMAKE_CXX_COMPILER=your-sycl-compiler
```

### 2. Build

```bash
# CMake
cmake -DCMAKE_BUILD_TYPE=Release -S ./src -B ./build
cmake --build ./build --config Release --target all --
```

### 3. Run

#### Interactive:
```bash
bin/Assignment2
```

#### Automated:
```bash
python3 scripts/run_measurements.py --bin bin/Assignment2 --out measurements.csv
```
With energy measurements:
```bash
sudo python3 scripts/run_measurements.py --bin bin/Assignment2 --out measurements.csv

```

**Note:** The script can be run with different arguments to control what is executed. Common options include *--options 1,2,3,4,5* (menu options), *--sizes 1024,2048,3072* (matrix size), *--block-sizes 32,64,128* (block sizes), *--threads 1,2,4,8,16* (OpenMP threads), and *--runs 3* (repetitions per configuration).


