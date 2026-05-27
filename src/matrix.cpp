#include <random>
#include "matrix.hpp"

// std::random_device rd;
// std::mt19937 gen(rd());
std::mt19937 gen(69420);
std::uniform_real_distribution<> dis(0.0, 10.0); // TODO: Consider using a larger range

/**
 * @brief Returns a random number in range of [0.0, 10.0]
 * @return double
 */
double rnd() {
    return dis(gen);
}

auto startOrResetMatrices(
    unsigned n,
    Matrix<double>& A,
    double*& b,
    double*& x,
    double*& y
) -> void {
    b = new double[n];
    x = new double[n];
    y = new double[n];

    for (unsigned i = 0; i < n; i++) {
        for (unsigned j = 0; j < n; j++) {
            A.set(i, j, rnd());
        }
        b[i] = rnd();
    }

    // TODO: Remove (DEBUG)
    // A.print("A");
    // std::cout << "b[*]: ";
    // for (unsigned i = 0; i < n; i++) {
    //     std::cout << b[i] << " ";
    // }
    // std::cout << std::endl;
}

auto freeMatrices(
    const double* b,
    const double* x,
    const double* y
) -> void {
    delete[] b;
    delete[] x;
    delete[] y;
}

auto solve(
    Matrix<double>& A,
    const double* const b,
    double* const x,
    double* const y
) -> void {
    // Forward Substitution:
    for (unsigned i = 0; i < A.size; i++) {
        double sum = 0.0;
        for (unsigned j = 0; j < i; j++)
            sum += A.get(i, j) * y[j];
        y[i] = b[i] - sum; // L[i*n + i] == 1.0 for Doolittle
    }

    // Backward Substitution:
    for (unsigned i = A.size; i-- > 0;) {
        double sum = 0.0;
        for (unsigned j = i + 1; j < A.size; j++)
            sum += A.get(i, j) * x[j];
        x[i] = (y[i] - sum) / A.get(i, i);
    }
}
