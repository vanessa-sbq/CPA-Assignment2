# CPA Assignment 2 - Shared Memory LU Factorization

This project contains the implementations for LU factorization in four variants:

1. Sequential LU
2. Block-oriented sequential LU
3. Shared-memory OpenMP LU
4. SYCL LU

## Build

### With Makefile

```bash
make
```

### With CMake

Configure the project with CMake:

```bash
cmake -DCMAKE_BUILD_TYPE:STRING=Release --no-warn-unused-cli -S ./src -B ./build
```

Build:

```bash
cmake --build ./build --config Release --target all --
```

## Run

```bash
./Assignment2
```

> **Note:** If you built with CMake, run the executable from the `bin` directory.

The program shows a menu where you can pick the implementation and enter the matrix size $n$ (for $n \times n$).
For block LU you will be prompted for the block size, and for OpenMP you will be prompted for the thread count.

## Performance Experiments

- Matrix sizes: $n \in \{1024, 2048, \ldots, 8192\}$
- Complexity: $\Theta(\frac{2}{3} n^3)$ flops
