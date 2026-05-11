#ifndef __LU_FACTORIZATION__
#define __LU_FACTORIZATION__

/**
 * TODO: Implement sequential LU factorization.
 */
void lu_fact_sequential(int n);

/**
 * TODO: Implement block-oriented LU factorization.
 */
void lu_fact_block(int n, int block_size);

/**
 * TODO: Implement shared-memory LU factorization using OpenMP.
 */
void lu_fact_omp(int n, int num_threads);

/**
 * TODO: Implement SYCL LU factorization.
 */
void lu_fact_sycl(int n);

#endif // __LU_FACTORIZATION__
