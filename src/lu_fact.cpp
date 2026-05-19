#include <algorithm>
#include <iostream>
#include <omp.h>
#include "lu_fact.hpp"

auto lufact::sequential(Matrix<double>& A) -> void {
    for (unsigned k = 0; k < A.size - 1; k++) {
        if (A(k, k) == 0) {
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
    for (unsigned k = 0; k < A.size - 1; k++) {
        if (A(k, k) == 0) {
            std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
            std::exit(1);
        }

        // Compute column k:
        unsigned vertical_blocks = (A.size + block_size - 1) / block_size; // Equivalent to roundup(A.size / block_size)
        for (unsigned b = 0; b < vertical_blocks; b++) { // For each block
            for ( // i is the line where the block starts (cannot be < k+1 neither >= A.size)
                unsigned i = std::max(k+1, b * block_size);
                i < std::min(A.size, (b+1) * block_size);
                i++
            ) {
                A(i, k) /= A(k, k);
            }
        }

        // Compute matrix A[k+1..size, k+1..size]:
        unsigned n_blocks = vertical_blocks * vertical_blocks;
        for (unsigned b = 0; b < n_blocks; b++) { // For each block
            unsigned bi = b / vertical_blocks;
            unsigned bj = b % vertical_blocks;
            for ( // i is the line where the block starts (cannot be < k+1 neither >= A.size)
                unsigned i = std::max(k+1, bi * block_size);
                i < std::min(A.size, (bi+1) * block_size);
                i++
            ) {
                for ( // j is the column where the block starts (cannot be < k+1 neither >= A.size)
                    unsigned j = std::max(k+1, bj * block_size);
                    j < std::min(A.size, (bj+1) * block_size);
                    j++
                ) {
                    A(i, j) -= A(i, k) * A(k, j);
                }
            }
        }
    }
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
