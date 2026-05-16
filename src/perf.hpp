#ifndef __PERF__
#define __PERF__

#include <functional>
#include "matrix.hpp"

typedef std::function<void(Matrix<>&)> Alg;

#define POWERCAP_PATH "/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj"

/**
 * Reads energy consumption in microjoules from the specified file path.
 * @return The energy consumption in microjoules.
 */
auto read_energy_uj() -> long long;

/**
 * Displays the execution time, GFlop/s, and energy consumption for LU factorization.
 * @param start The start time of the operation.
 * @param end The end time of the operation.
 * @param n Number of rows/columns of matrix A
 * @param e_before Energy consumption before the operation (in microjoules)
 * @param e_after Energy consumption after the operation (in microjoules)
 */
auto display_measurements(
    double start,
    double end,
    unsigned n,
    double e_before,
    double e_after
) -> void;

/**
 * Solves a Matrix equation (`Ax=b`) with a given LU factorization algorithm and measures its performance
 * @param f LU factorization algorithm
 * @param n size of vectors b, x
 */
auto perform(const Alg &f, unsigned n) -> void;

/**
 * Measures the performance of an LU factorization algorithm
 * @param f LU factorization algorithm
 * @param A matrix A in equation of type `Ax=b`
 */
auto measure(const Alg &f, Matrix<>& A) -> void;

#endif // __PERF__
