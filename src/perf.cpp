#include <fstream>
#include <iostream>
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
    printf("\nTime: %g seconds\n", executionTime);

    // 2/3 * n^3 flops per factorization
    double gflops = (2.0 / 3.0) * n * n * n / (executionTime * 1e9);
    printf("GFlop/s: %g\n", gflops);

    double joules = (e_after - e_before) / 1e6;
    double watts = joules / executionTime;
    printf("Joules: %.6f\n", joules);
    printf("Watts: %.6f\n", watts);
}

auto perform(const Alg& f, unsigned n, sycl::queue *q) -> void {
    (void)q;
    Matrix<> A(n);
    double *b, *x, *y;
    startOrResetMatrices(n, A, b, x, y);

    A.preview("A");
    std::cout << "b[*]: ";
    for (unsigned i = 0; i < std::min(10u, n); i++) {
        std::cout << b[i] << " ";
    }
    std::cout << std::endl;

    if (q) {
        Matrix<double> A_device(A.size, q);
        double start_host2dev = omp_get_wtime();
        q->memcpy(A_device.get_buf(), A.get_buf(), A.size * A.size * sizeof(double)).wait();
        double host2dev_time = omp_get_wtime() - start_host2dev;

        measure(f, A_device);

        double start_dev2host = omp_get_wtime();
        q->memcpy(A.get_buf(), A_device.get_buf(), A.size * A.size * sizeof(double)).wait();
        double dev2host_time = omp_get_wtime() - start_dev2host;
        
        printf(
            "Memory transfer overheads:\n"
            "Host to device: %.3gs\n"
            "Device to host: %.3gs\n"
            , host2dev_time, dev2host_time
        );
    } else {
        measure(f, A);
    }
    
    solve(A, b, x, y);
    
    A.preview("LU");
    std::cout << "x[*]: ";
    for (unsigned i = 0; i < std::min(10u, n); i++) {
        std::cout << x[i] << " ";
    }
    std::cout << std::endl;

    freeMatrices(b, x, y);
}

auto measure(const Alg& f, Matrix<double>& A) -> void {
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
