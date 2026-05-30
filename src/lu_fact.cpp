#include <algorithm>
#include <omp.h>
#include <stdexcept>
#include <sycl/sycl.hpp>
#include "lu_fact.hpp"

auto lufact::sequential(Matrix<double>& A) -> void {
    Matrix<unsigned> count(A.size);
    for (unsigned k = 0; k < A.size - 1; k++) {
        // if (A(k, k) == 0) {
        //     std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
        //     std::exit(1);
        // }

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
        // if (A(k, k) == 0) {
        //     std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
        //     std::exit(1);
        // }

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

auto lufact::omp(Matrix<double>& A, unsigned num_threads, unsigned block_size) -> void {
    
    for (unsigned k = 0; k < A.size - 1; k++) {
        // if (A(k, k) == 0) {
        //     std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
        //     std::exit(1);
        // }

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



auto lufact::sycl_dumb(Matrix<double>& A, unsigned const block_size, sycl::queue &q) -> void {
    const unsigned vertical_blocks = (A.size + block_size - 1) / block_size; // Equivalent to roundup(A.size / block_size)
    const unsigned n_blocks = vertical_blocks * vertical_blocks;

    double * const buf = A.get_buf();
    unsigned const size = A.size;

    for (unsigned k = 0; k < A.size - 1; k++) {
        // if (A(k, k) == 0) {
        //     std::cerr << "Error: value 0 found in matrix diagonal. Aborting..." << std::endl;
        //     std::exit(1);
        // }

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
        throw std::invalid_argument("Function currently only supports a size multiple of block_size");

    const unsigned vertical_blocks = (A.size + block_size - 1) / block_size; // Equivalent to roundup(A.size / block_size)
    [[maybe_unused]] const unsigned n_blocks = vertical_blocks * vertical_blocks;

    double * const buf = A.get_buf();
    unsigned const size = A.size;

    for (unsigned i = 0; i < vertical_blocks; ++i) {
        const unsigned ii0 = i*block_size;

        // diagonal update
        for (unsigned ii = ii0; ii < ii0+block_size; ++ii) {
            q.parallel_for(sycl::range<1>(ii0+block_size - ii - 1), [=](sycl::id<1> jj) {
                jj += ii + 1;
                buf[jj*size + ii] /= buf[ii*size + ii];
            });

            q.parallel_for(sycl::range<1>(ii0+block_size - ii - 1), [=](sycl::id<1> jj) {
                jj += ii + 1;
                for (unsigned kk = ii+1; kk < ii0+block_size; ++kk) {
                    buf[jj*size + kk] -= buf[jj*size + ii] * buf[ii*size + kk];
                }
            });
        }

        if (i+1 >= vertical_blocks)
            break;

        // lower TRSM
        for (unsigned j = i+1; j < vertical_blocks; ++j) {
            const unsigned jj0 = j*block_size;
            q.parallel_for(sycl::range<2>(block_size, block_size), [=](sycl::id<2> id) {
                const unsigned ii = ii0 + id[0];
                const unsigned jj = jj0 + id[1];
                for (unsigned kk = ii+1; kk < ii0+block_size; ++kk) {
                    buf[jj*size + kk] -= buf[jj*size + ii] * buf[ii*size + kk];
                }
            });
        }

        // upper TRSM
        for (unsigned j = i+1; j < vertical_blocks; ++j) {
            const unsigned jj0 = j*block_size;
            q.parallel_for(sycl::range<2>(block_size, block_size), [=](sycl::id<2> id) {
                const unsigned ii = ii0 + id[0];
                const unsigned jj = jj0 + id[1];
                buf[jj*size + ii] /= buf[ii*size + ii];
            });

            q.parallel_for(sycl::range<2>(block_size, block_size), [=](sycl::id<2> id) {
                const unsigned ii = ii0 + id[0];
                const unsigned jj = jj0 + id[1];
                for (unsigned kk = ii + 1; kk < ii0+block_size; ++kk) {
                    buf[kk*size + jj] -= buf[kk*size + ii] * buf[ii*size + jj];
                }
            });
        }

        const unsigned k = i*block_size;
        const unsigned k1 = k+block_size;
        // const unsigned bsize = k1+block_size <= size ? block_size : size-k1;
        const unsigned bsize = block_size;

        // trailing updates (GEMM)
        q.submit([&](sycl::handler &h) {
            sycl::local_accessor<double,2> L({bsize,bsize}, h);
            sycl::local_accessor<double,2> U({bsize,bsize}, h);

            h.parallel_for(sycl::nd_range<2>(
                sycl::range<2>(size - k1, size - k1),
                sycl::range<2>(bsize, bsize)
                // sycl::id<2>(k1, k1)
            ), [=](sycl::nd_item<2> it) {
                const unsigned
                    li = it.get_local_id(0),
                    lj = it.get_local_id(1),
                    gi = k1+it.get_global_id(0),
                    gj = k1+it.get_global_id(1);

                // copy matrices to local/shared memory
                L[li][lj] = buf[gi*size + (k+lj)];
                U[lj][li] = buf[(k+li)*size + gj];

                it.barrier(); // ensure matrices are ready

                // Calculate 
                double acc = 0;
                #pragma unroll
                for (unsigned i = 0; i < bsize; ++i) {
                    acc += L[li][i] * U[lj][i];
                }

                buf[gi*size + gj] -= acc;

            });
        });
    }
}
