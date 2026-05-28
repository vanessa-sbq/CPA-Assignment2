#include <algorithm>
#include <cmath>
#include <iostream>
#include <omp.h>
#include "lu_fact.hpp"


/*
 * Performs the LU factorization of a matrix A using the sequential algorithm.
 * The result is stored in the same matrix A, where the lower triangular part contains L (with 1s in the diagonal) and the upper triangular part contains U.
 * @param A The matrix to be factored. It will be modified to contain the LU decomposition.
*/
auto lufact::sequential(Matrix<double>& A) -> void {
    Matrix<unsigned> count(A.size);
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


/*
 * Performs the LU factorization of a matrix A using the block algorithm.
 * The result is stored in the same matrix A, where the lower triangular part contains L (with 1s in the diagonal) and the upper triangular part contains U.
 * @param A The matrix to be factored. It will be modified to contain the LU decomposition.
 * @param block_size The size of the blocks to be used in the factorization. It should be a positive integer less than or equal to A.size.
*/
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


/*
 * Performs the LU factorization of a matrix A using the OpenMP parallel algorithm.
 * The result is stored in the same matrix A, where the lower triangular part contains L (with 1s in the diagonal) and the upper triangular part contains U.
 * @param A The matrix to be factored. It will be modified to contain the LU decomposition.
 * @param num_threads The number of threads to use for parallel execution.
 * @param block_size The size of the blocks to be used in the factorization. It should be a positive integer less than or equal to A.size.
 */
auto lufact::omp(Matrix<double>& A, unsigned num_threads, unsigned block_size) -> void {
    for (unsigned k = 0; k < A.size - 1; k++) {
        if (A(k, k) == 0) {
            std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
            std::exit(1);
        }

        // Compute column k:
        unsigned vertical_blocks = (A.size + block_size - 1) / block_size; // Equivalent to roundup(A.size / block_size)
        #pragma omp parallel for schedule(dynamic) num_threads(num_threads)
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
        #pragma omp parallel for schedule(dynamic) num_threads(num_threads)
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

auto lufact::sycl(Matrix<double>& A) -> void {
    (void) A;
    // TODO
}


/*
 * Verifies the correctness of the LU decomposition by checking if L * U equals the original matrix.
 * @param A_lu The LU decomposed matrix.
 * @param A_original The original matrix.
 * @param tol The tolerance for the verification.
 * @return True if the verification passes, false otherwise.
 */
auto lufact::debug_verify(const Matrix<double>& A_lu, const Matrix<double>& A_original, double tol) -> bool {
    // If the sizes differ, we cannot compare the matrices, so we consider it a verification failure
    if (A_lu.size != A_original.size) {
        std::cerr << "Debug verify error: matrix sizes differ." << std::endl;
        return false;
    }

    // Compute the maximum absolute error between A_original and L*U (where L and U are obtained from A_lu)
    double max_abs_err = 0.0;
    for (unsigned i = 0; i < A_lu.size; i++) {
        for (unsigned j = 0; j < A_lu.size; j++) {
            double sum = 0.0;
            for (unsigned k = 0; k < A_lu.size; k++) {
                double l = (i == k) ? 1.0 : (i > k ? A_lu.get(i, k) : 0.0);
                double u = (k <= j) ? A_lu.get(k, j) : 0.0;
                sum += l * u;
            }
            double diff = std::abs(sum - A_original.get(i, j));
            max_abs_err = std::max(max_abs_err, diff);
        }
    }

    // Print the maximum absolute error and whether the verification passed or failed based on the tolerance
    std::cout << "LU verify max abs error: " << max_abs_err << std::endl;
    if (max_abs_err > tol) {
        std::cout << "LU verify FAILED (tol=" << tol << ")" << std::endl;
        return false;
    }

    std::cout << "LU verify OK (tol=" << tol << ")" << std::endl;
    return true;
}
