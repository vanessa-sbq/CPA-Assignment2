# CPA Assignment 2 - Shared Memory LU Factorization

This project contains the implementations for LU factorization in four variants:

1. Sequential LU
2. Block-oriented sequential LU
3. Shared-memory OpenMP LU
4. SYCL LU

## Build

Configure the project with CMake (replace `your-c++-sycl-compiler` with your SYCL compiler (e.g. `acpp` ([AdaptiveCpp](https://github.com/AdaptiveCpp/AdaptiveCpp)) or `icpx` ([Intel oneAPI DPC++/C++](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html)))):

```bash
cmake -DCMAKE_BUILD_TYPE:STRING=Release --no-warn-unused-cli -S ./src -B ./build -DCMAKE_CXX_COMPILER=your-sycl-compiler
```

Build:

```bash
cmake --build ./build --config Release --target all --
```

## Run

```bash
bin/Assignment2
```

The program shows a menu where you can pick the implementation and enter the matrix size $n$ (for $n \times n$).
For block LU you will be prompted for the block size, and for OpenMP you will be prompted for the thread count.

## Performance Experiments

- Matrix sizes: $n \in \{1024, 2048, \ldots, 8192\}$
- Complexity: $\Theta(\frac{2}{3} n^3)$ flops
