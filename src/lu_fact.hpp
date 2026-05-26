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
    auto sycl(Matrix<double>& A, unsigned block_size, sycl::queue &q) -> void;
}

#endif // __LU_FACTORIZATION__
