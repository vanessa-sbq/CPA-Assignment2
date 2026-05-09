#include <omp.h>
#include "matrix.hpp"

void lu_fact_sequential(int n) {
    /// Note: implementation based from this: https://www.geeksforgeeks.org/dsa/doolittle-algorithm-lu-decomposition/
    for (int i = 0; i < n; i++) {
        // Upper Triangular
        for (int k = i; k < n; k++) {
            // Summation of L(i, j) * U(j, k)
            double sum = 0.0;
            for (int j = 0; j < i; j++)
                sum += (L[i*n + j] * U[j * n + k]);

            U[i * n + k] = A[i * n + k] - sum;
        }

        // Lower Triangular
        for (int k = i; k < n; k++) {
            if (i != k) {
                // Summation of L(k, j) * U(j, i)
                double sum = 0.0;
                for (int j = 0; j < i; j++)
                    sum += (L[k * n + j] * U[j * n + i]);

                // Evaluating L(k, i)
                L[k * n + i] = (A[k * n + i] - sum) / U[i * n + i];
            }
        }
    }
}

void lu_fact_block(int n, int block_size) {
    (void) n;
    (void)block_size;
    // TODO
}

void lu_fact_omp(int n, int num_threads) {
    (void) n;
    (void) num_threads;
    // TODO
}

void lu_fact_sycl(int n) {
    (void) n;
    // TODO
}
