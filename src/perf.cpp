#include <fstream>
#include <iostream>
#include <print>
#include <omp.h>
#include "perf.hpp"

auto read_energy_uj() -> long long {
    std::ifstream f(POWERCAP_PATH); // directory path where the energy_uj file is located.
    if (f.fail())
        return 0;

    // This part is commented for the benchmark on FEUP's PCs
    // if (f.fail()) { 
    // 	fprintf(stderr, "Failed to open %s: %s\n", POWERCAP_PATH, strerror(errno));
    // 	if (errno == ENOENT) {
    // 		fprintf(stderr, "Is powercap installed?\n");
    // 	} else if (errno == EACCES) {
    // 		fprintf(stderr, "Try running with sudo to get energy information\n");
    // 		return 0;
    // 	}
    // 	exit(1);
    // }

    long long val; f >> val;
    return val;
}

auto display_measurements(
    double start,
    double end,
    unsigned n,
    double e_before,
    double e_after
) -> void {
    double executionTime = (end - start);
    std::println("Time: {} seconds", executionTime);

    // 2/3 * n^3 flops per factorization
    double gflops = (2.0 / 3.0) * n * n * n / (executionTime * 1e9);
    std::println("GFlop/s: {}", gflops);

    double joules = (e_after - e_before) / 1e6;
    double watts = joules / executionTime;
    std::println("Joules: {:.6f}", joules);
    std::println("Watts: {:.6f}", watts);
}

auto perform(const Alg &f, unsigned n) -> void {
    Matrix<> A(n);
    double *b, *x, *y;
    startOrResetMatrices(n, A, b, x, y);
    
    measure(f, A);
    
    solve(A, b, x, y);
    
    A.preview("A");
    std::cout << "x[*]: ";
    for (unsigned i = 0; i < std::min(10u, n); i++) {
        std::cout << x[i] << " ";
    }
    std::cout << std::endl;

    freeMatrices(b, x, y);
}

auto measure(const Alg &f, Matrix<>& A) -> void {
    auto e_before = read_energy_uj();
    double start = omp_get_wtime(); // Get start time

    f(A);

    double end = omp_get_wtime(); // Get end time
    auto e_after = read_energy_uj();

    display_measurements(
        start,
        end,
        A.size,
        (double) e_before,
        (double) e_after
    );
}
