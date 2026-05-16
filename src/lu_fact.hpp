#ifndef __LU_FACTORIZATION__
#define __LU_FACTORIZATION__

#include "matrix.hpp"

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
     */
    auto omp(Matrix<double>& A, unsigned num_threads) -> void;

    /**
     * Performs in-place LU factorization of a matrix using SYCL
     * @param A Square matrix to be factorized
     */
    auto sycl(Matrix<double>& A) -> void;
}

#endif // __LU_FACTORIZATION__
