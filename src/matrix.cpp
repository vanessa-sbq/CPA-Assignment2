#include <iostream>
#include <random>
#include "matrix.hpp"

double *A, *L, *U, *x, *y, *b;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 10.0); // TODO: Consider using a larger range

/**
 * @brief Returns a random number in range of [0.0, 10.0]
 * @return double
 */
double rnd() {
    return dis(gen);
}

void print_matrix(double *M, int n, const char* name) {
    std::cout << name << " = [" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "  ";
        for (int j = 0; j < n; j++) {
            std::cout << M[i*n + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

void startOrResetMatrices(int n) {
    A = new double[n * n];
    L = new double[n * n];
    U = new double[n * n];
    b = new double[n];
    x = new double[n];
    y = new double[n];

    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            A[i * n + j] = rnd();
            L[i * n + j] = (i == j) ? 1.0 : 0.0;
            U[i * n + j] = 0.0;
        }
        b[i] = rnd();
    }

    // TODO: Remove (DEBUG)
    // print_matrix(A, n, "A");
    // print_matrix(L, n, "L");
    // print_matrix(U, n, "U");
    // print_matrix(b, n, "b");
}

void freeMatrices() {
    delete[] A;
    delete[] L;
    delete[] U;
    delete[] x;
    delete[] y;
    delete[] b;
}

void solve(int n) {
    // Forward Substitution:
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < i; j++)
            sum += L[i * n + j] * y[j];
        y[i] = b[i] - sum; // L[i*n + i] == 1.0 for Doolittle
    }

    // Backward Substitution:
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0.0;
        for (int j = i + 1; j < n; j++)
            sum += U[i * n + j] * x[j];
        x[i] = (y[i] - sum) / U[i * n + i];
    }
}

void show_result_matrix(int n) {
    int j;
    std::cout << "L[0,*]: ";
    for (j = 0; j < std::min(10, n); j++) {
        std::cout << L[j] << " ";
    }
    std::cout << std::endl;
    std::cout << "U[0,*]: ";
    for (j = 0; j < std::min(10, n); j++) {
        std::cout << U[j] << " ";
    }
    std::cout << std::endl;
    std::cout << "x[*]: ";
    for (j = 0; j < std::min(10, n); j++) {
        std::cout << x[j] << " ";
    }
    std::cout << std::endl;
}
