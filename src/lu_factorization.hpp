#ifndef __LU_FACTORIZATION__
#define __LU_FACTORIZATION__

#include "matrix.hpp"

/**
 * TODO: Implement sequential LU factorization.
 */
auto lu_fact_sequential(Matrix<double>& A) -> void;

/**
 * TODO: Implement block-oriented LU factorization.
 */
auto lu_fact_block(Matrix<double>& A, unsigned block_size) -> void;

/**
 * TODO: Implement shared-memory LU factorization using OpenMP.
 */
auto lu_fact_omp(Matrix<double>& A, unsigned num_threads) -> void;

/**
 * TODO: Implement SYCL LU factorization.
 */
auto lu_fact_sycl(Matrix<double>& A) -> void;

#endif // __LU_FACTORIZATION__
