#include <iostream>
#include <cstdlib>
#include "matrix.hpp"

double *A, *L, *U;

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
	A = (double *)malloc((n * n) * sizeof(double));
	L = (double *)malloc((n * n) * sizeof(double));
	U = (double *)malloc((n * n) * sizeof(double));

	int i, j;

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			A[i * n + j] = (i == j) ? (double)n : 1.0; // FIXME: Not sure if this should be initialized like this
			L[i * n + j] = (i == j) ? 1.0 : 0.0;
			U[i * n + j] = 0.0;
		}
	}

    // TODO: Remove (DEBUG)
    print_matrix(A, n, "A");
    print_matrix(L, n, "L");
    print_matrix(U, n, "U");
}

void freeMatrices() {
	free(A);
	free(L);
	free(U);
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
}
