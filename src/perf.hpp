#ifndef __PERF__
#define __PERF__

#include <functional>

#define POWERCAP_PATH "/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj"

/**
 * Reads energy consumption in microjoules from the specified file path.
 * @return The energy consumption in microjoules.
 */
long long read_energy_uj();

/**
 * Displays the execution time, GFlop/s, and energy consumption for LU factorization.
 * @param start The start time of the operation.
 * @param end The end time of the operation.
 * @param n Number of rows/columns of matrix A
 * @param e_before Energy consumption before the operation (in microjoules)
 * @param e_after Energy consumption after the operation (in microjoules)
 */
void display_measurements(double start, double end, int n, double e_before, double e_after);

/**
 * Measures the performance of a function
 * @param f The function to be measured
 * @param n Number of rows/columns of a matrix
 */
void measure(const std::function<void(int)> &f, int n);

#endif // __PERF__
