#ifndef __LU_FACTORIZATION__
#define __LU_FACTORIZATION__

#include "matrix.hpp"
#include <sycl/sycl.hpp>

namespace lufact {
    /**
     * Performs in-place sequential LU factorization of a matrix
     * @param A Square matrix to be factorized
     */
    auto sequential(Matrix<double>& A) -> void;

    /**
     * Performs in-place block-oriented LU factorization of a matrix
     * @param A Square matrix to be factorized
     * @param block_size Size of the blocks used in the block algorithm
     */
    auto block(Matrix<double>& A, unsigned block_size) -> void;

    /**
     * Performs in-place parallel LU factorization of a matrix using OpenMP
     * @param A Square matrix to be factorized
     * @param num_threads Number of OpenMP threads to use for parallelization
     * @param block_size Size of the blocks used in the block algorithm
     */
    auto omp(Matrix<double>& A, unsigned num_threads, unsigned block_size) -> void;

    /**
     * Performs in-place LU factorization of a matrix using SYCL
     * @param A Square matrix to be factorized
     */
    auto sycl_dumb(Matrix<double>& A, unsigned block_size, sycl::queue &q) -> void;
    auto sycl_basic(Matrix<double>& A, unsigned block_size, sycl::queue &q) -> void;
    auto sycl_block(Matrix<double>& A, unsigned block_size, sycl::queue &q) -> void;

    /**
     * Debug helper to verify LU factorization by checking that L*U ~= A_original
     * @param A_lu Matrix containing in-place LU (Doolittle: unit diagonal in L)
     * @param A_original Original matrix before factorization
     * @param tol Absolute tolerance for the maximum error
     * @return true if max absolute error <= tol
     */
    auto debug_verify(const Matrix<double>& A_lu, const Matrix<double>& A_original, double tol = 1e-9) -> bool;
}

#endif // __LU_FACTORIZATION__
