#include <iostream>
#include <omp.h>
#include "lu_fact.hpp"

auto lufact::sequential(Matrix<double>& A) -> void {
    for (unsigned k = 0; k < A.size - 1; k++) {
        if (A(k, k) != 0) {
            std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
            std::exit(1);
        }

        for (unsigned i = k+1; i < A.size; i++) {
            A(i, k) /= A(k, k);
        }

        for (unsigned i = k+1; i < A.size; i++) {
            for (unsigned j = k+1; j < A.size; j++) {
                A(i, j) -= A(i, k) * A(k, j);
            }
        }
    }
}

auto lufact::block(Matrix<double>& A, unsigned block_size) -> void {
    (void) A;
    (void)block_size;
    // TODO
}

auto lufact::omp(Matrix<double>& A, unsigned num_threads) -> void {
    (void) A;
    (void) num_threads;
    // TODO
}

auto lufact::sycl(Matrix<double>& A) -> void {
    (void) A;
    // TODO
}
