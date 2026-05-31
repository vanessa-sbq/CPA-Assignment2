#include <algorithm>
#include <omp.h>
#include <stdexcept>
#include <sycl/sycl.hpp>
#include <cmath>
#include <iostream>
#include "lu_fact.hpp"


auto lufact::sequential(Matrix<double>& A) -> void {
    Matrix<unsigned> count(A.size);
    for (unsigned k = 0; k < A.size - 1; k++) {
        // Check to avoid division by zero
        if (A(k, k) == 0) {
            std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
            std::exit(1);
        }        

        // Compute column k (divide each element by the pivot)
        for (unsigned i = k+1; i < A.size; i++) { 
            A(i, k) /= A(k, k);
        }

        // Compute matrix A[k+1..size, k+1..size] (update the trailing submatrix)
        for (unsigned i = k+1; i < A.size; i++) { 
            for (unsigned j = k+1; j < A.size; j++) {
                A(i, j) -= A(i, k) * A(k, j);
            }
        }
    }
}


auto lufact::block(Matrix<double>& A, unsigned block_size) -> void {
    const unsigned n = A.size;

    for (unsigned k = 0; k < n; k += block_size) {
        const unsigned k1 = std::min(n, k + block_size);

        // Check to avoid division by zero
        if (A(k, k) == 0) {
            std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
            std::exit(1);
        }

        // Run standard sequential LU factorization on A[k:k1, k:k1] (produces a small L and U in-place)
        for (unsigned p = k; p < k1 - 1; p++) {
            for (unsigned i = p+1; i < k1; i++)
                A(i, p) /= A(p, p);
            for (unsigned i = p+1; i < k1; i++)
                for (unsigned j = p+1; j < k1; j++)
                    A(i, j) -= A(i, p) * A(p, j);
        }

        if (k1 >= n) break; // No more blocks to process

        // Compute block column below the pivot tile (L[k1:n, k:k1])
        for (unsigned p = k; p < k1; p++) { // Solve X * U[k:k1, k:k1] = A[k1:n, k:k1] in-place
            for (unsigned i = k1; i < n; i++) 
                A(i, p) /= A(p, p);
            for (unsigned i = k1; i < n; i++)
                for (unsigned j = p+1; j < k1; j++)
                    A(i, j) -= A(i, p) * A(p, j);
        }

        // Compute block row to the right of the pivot tile (U[k:k1, k1:n])
        for (unsigned p = k; p < k1; p++) // Solve L[k:k1, k:k1] * X = A[k:k1, k1:n] in-place (L has diagonal equal to 1)
            for (unsigned i = p+1; i < k1; i++)
                for (unsigned j = k1; j < n; j++)
                    A(i, j) -= A(i, p) * A(p, j);

        // Update bottom right of the tile (A[k1:n, k1:n] -= L[k1:n, k:k1] * U[k:k1, k1:n])
        for (unsigned i = k1; i < n; i++)
            for (unsigned p = k; p < k1; p++)
                for (unsigned j = k1; j < n; j++)
                    A(i, j) -= A(i, p) * A(p, j);
    }
}


auto lufact::omp(Matrix<double>& A, unsigned num_threads, unsigned block_size) -> void {
    const unsigned n = A.size;

    for (unsigned k = 0; k < n; k += block_size) {
        const unsigned k1 = std::min(n, k + block_size);

        // Panel factorization (inherently sequential)
        for (unsigned p = k; p < k1 - 1; p++) {
            for (unsigned i = p+1; i < k1; i++)
                A(i, p) /= A(p, p);
            for (unsigned i = p+1; i < k1; i++)
                for (unsigned j = p+1; j < k1; j++)
                    A(i, j) -= A(i, p) * A(p, j);
        }

        if (k1 >= n) break;

        #pragma omp parallel num_threads(num_threads)
        {
            // Compute block column below the pivot tile (L[k1:n, k:k1])
            for (unsigned p = k; p < k1; p++) {
                #pragma omp for schedule(dynamic)
                for (unsigned i = k1; i < n; i++)
                    A(i, p) /= A(p, p);
                #pragma omp for schedule(dynamic)
                for (unsigned i = k1; i < n; i++)
                    for (unsigned j = p+1; j < k1; j++)
                        A(i, j) -= A(i, p) * A(p, j);
            }

            // Compute block row to the right of the pivot tile (U[k:k1, k1:n])
            for (unsigned p = k; p < k1; p++)
                #pragma omp for schedule(dynamic)
                for (unsigned i = p+1; i < k1; i++)
                    for (unsigned j = k1; j < n; j++)
                        A(i, j) -= A(i, p) * A(p, j);

            // Update bottom right of the tile (A[k1:n, k1:n] -= L[k1:n, k:k1] * U[k:k1, k1:n])
            #pragma omp for schedule(dynamic)
            for (unsigned i = k1; i < n; i++)
                for (unsigned p = k; p < k1; p++)
                    for (unsigned j = k1; j < n; j++)
                        A(i, j) -= A(i, p) * A(p, j);
        }
    }
}


auto lufact::sycl_dumb(Matrix<double>& A, unsigned const block_size, sycl::queue &q) -> void {
    const unsigned vertical_blocks = (A.size + block_size - 1) / block_size; // Equivalent to roundup(A.size / block_size)
    const unsigned n_blocks = vertical_blocks * vertical_blocks;

    double * const buf = A.get_buf();
    unsigned const size = A.size;

    for (unsigned k = 0; k < A.size - 1; k++) {
        q.parallel_for(sycl::range<1>(vertical_blocks), [buf, block_size, size, k](sycl::id<1> id){
            unsigned b = id;
            for ( // i is the line where the block starts (cannot be < k+1 nor >= A.size)
                unsigned i = std::max(k+1, b * block_size);
                i < std::min(size, (b+1) * block_size);
                i++
            ) {
                buf[i*size + k] /= buf[k*size + k];
                // A(i, k) /= A(k, k);
            }
        });

        q.parallel_for(sycl::range<1>(n_blocks), [buf, vertical_blocks, block_size, size, k](sycl::id<1> id) {
            unsigned b = id;
            unsigned bi = b / vertical_blocks;
            unsigned bj = b % vertical_blocks;
            for ( // i is the line where the block starts (cannot be < k+1 nor >= A.size)
                unsigned i = std::max(k+1, bi * block_size);
                i < std::min(size, (bi+1) * block_size);
                i++
            ) {
                for ( // j is the column where the block starts (cannot be < k+1 nor >= A.size)
                    unsigned j = std::max(k+1, bj * block_size);
                    j < std::min(size, (bj+1) * block_size);
                    j++
                ) {
                    buf[i*size + j] -= buf[i*size + k] * buf[k*size + j];
                    // A(i, j) -= A(i, k) * A(k, j);
                }
            }
        });
    }
}



auto lufact::sycl_basic(Matrix<double>& A, [[maybe_unused]] unsigned const block_size, sycl::queue &q) -> void {
    double * const buf = A.get_buf();
    unsigned const size = A.size;

    for (unsigned k = 0; k < A.size - 1; k++) {
        q.parallel_for(sycl::range<1>(size-k-1), [buf, size, k](sycl::id<1> i) {
            i += k+1;
            buf[i*size + k] /= buf[k*size + k];
        });

        q.parallel_for(sycl::range<2>(size-k-1, size-k-1), [buf, size, k](sycl::id<2> id) {
            unsigned i = k+1+id[0], j = k+1+id[1];
            buf[i*size + j] -= buf[i*size + k] * buf[k*size + j];
        });
    }
}



auto lufact::sycl_block(Matrix<double>& A, unsigned block_size, sycl::queue &q) -> void {
    block_size = 32;

    if (A.size % block_size != 0)
        throw std::invalid_argument("sycl_block currently only supports sizes that are a multiple of block_size");

    double * const buf = A.get_buf();
    const unsigned size  = A.size;
    const unsigned bsize = block_size;

    for (unsigned k = 0; k < size; k += block_size) {
        const unsigned k1 = k + block_size; // == min(size, k+block_size) since size % block_size == 0

        // 1. panel factorization of the diagonal block A[k:k1, k:k1] (parallel over rows, sequential pivot)
        for (unsigned p = k; p < k1 - 1; ++p) {
            q.parallel_for(sycl::range<1>(k1 - p - 1), [=](sycl::id<1> id) {
                const unsigned i = p + 1 + id;
                buf[i*size + p] /= buf[p*size + p];
            });
            q.parallel_for(sycl::range<1>(k1 - p - 1), [=](sycl::id<1> id) {
                const unsigned i = p + 1 + id;
                for (unsigned j = p + 1; j < k1; ++j)
                    buf[i*size + j] -= buf[i*size + p] * buf[p*size + j];
            });
        }

        if (k1 >= size) break; // No more blocks to process

        // 2. lower TRSM, L[k1:n, k:k1] (each work item owns one row; pivot loop sequential, with divide)
        q.parallel_for(sycl::range<1>(size - k1), [=](sycl::id<1> id) {
            const unsigned i = k1 + id;
            for (unsigned p = k; p < k1; ++p) {
                buf[i*size + p] /= buf[p*size + p];
                for (unsigned j = p + 1; j < k1; ++j)
                    buf[i*size + j] -= buf[i*size + p] * buf[p*size + j];
            }
        });

        // 3. upper TRSM, U[k:k1, k1:n] (each work item owns one column; pivot loop sequential, no divide)
        q.parallel_for(sycl::range<1>(size - k1), [=](sycl::id<1> id) {
            const unsigned j = k1 + id;
            for (unsigned p = k; p < k1; ++p)
                for (unsigned i = p + 1; i < k1; ++i)
                    buf[i*size + j] -= buf[i*size + p] * buf[p*size + j];
        });

        // 4. trailing update (GEMM), A[k1:n, k1:n] -= L[k1:n, k:k1] * U[k:k1, k1:n] (tiled, local memory)
        q.submit([&](sycl::handler &h) {
            sycl::local_accessor<double,2> L({bsize, bsize}, h);
            sycl::local_accessor<double,2> U({bsize, bsize}, h);

            h.parallel_for(sycl::nd_range<2>(
                sycl::range<2>(size - k1, size - k1),
                sycl::range<2>(bsize, bsize)
            ), [=](sycl::nd_item<2> it) {
                const unsigned
                    li = it.get_local_id(0),
                    lj = it.get_local_id(1),
                    gi = k1 + it.get_global_id(0),
                    gj = k1 + it.get_global_id(1);

                // Copy the L and U tiles into local/shared memory
                L[li][lj] = buf[gi*size + (k + lj)]; // L[li][p] = A(gi, k+p)
                U[lj][li] = buf[(k + li)*size + gj]; // U[lj][p] = A(k+p, gj)

                it.barrier(); // ensure both tiles are ready

                double acc = 0;
                for (unsigned p = 0; p < bsize; ++p)
                    acc += L[li][p] * U[lj][p];

                buf[gi*size + gj] -= acc; // A(gi,gj) -= sum_p A(gi,p) * A(p,gj)
            });
        });
    }
}


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
