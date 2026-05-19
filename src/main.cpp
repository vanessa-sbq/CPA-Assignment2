#include <iostream>
#include <omp.h>
#include "perf.hpp"
#include "lu_fact.hpp"

inline auto sequential(unsigned n) -> void;
inline auto block(unsigned n) -> void;
inline auto omp(unsigned n) -> void;
inline auto sycl(unsigned n) -> void;

/**
 * Main function to execute the LU factorization based on user input.
 */
auto main () -> int {
    unsigned n = 0;
    unsigned char op = 1;

    do {
        std::cout << std::endl;
        std::cout << "1. Sequential LU" << std::endl;
        std::cout << "2. Block LU" << std::endl;
        std::cout << "3. OpenMP LU" << std::endl;
        std::cout << "4. SYCL LU" << std::endl;
        std::cout << "5. Exit" << std::endl;
        std::cout << "> ";
        std::cin >> op;
        if (op == 5) break;

        std::cout << "Matrix size n (n x n) ?" << std::endl;
        std::cout << "> ";
        std::cin >> n;
        if (n <= 0) {
            std::cout << "Invalid n." << std::endl;
            continue;
        }

        switch (op) {
            case 1: {
                sequential(n);
                break;
            }
            case 2: {
                block(n);
                break;
            }
            case 3: {
                omp(n);
                break;
            }
            case 4: {
                sycl(n);
                break;
            }
            default: {
                std::cout << "Unknown option." << std::endl;
                break;
            }
        }
    } while (op != 0);
    return 0;
}

auto sequential(unsigned n) -> void {
    perform(lufact::sequential, n);
}

auto block(unsigned n) -> void {
    unsigned block_size = 64;
    std::cout << "Block size ? " << std::endl;
    std::cout << "> ";
    std::cin >> block_size;
    if (block_size <= 0) {
        std::cout << "Invalid block size." << std::endl;
        return;
    }
    perform([block_size](Matrix<>& A) {
        lufact::block(A, block_size);
    }, n);
}

auto omp(unsigned n) -> void {
    int nt = 1;
    std::cout << "Number of threads? " << std::endl;
    std::cout << "> ";
    std::cin >> nt;
    if (nt <= 0) {
        std::cout << "Invalid thread count." << std::endl;
        return;
    }
    unsigned block_size = 64;
    std::cout << "Block size ? " << std::endl;
    std::cout << "> ";
    std::cin >> block_size;
    if (block_size <= 0) {
        std::cout << "Invalid block size." << std::endl;
        return;
    }
    perform([nt, block_size](Matrix<>& A) {
        omp_set_num_threads(nt);
        lufact::omp(A, nt, block_size);
    }, n);
}

auto sycl(unsigned n) -> void {
    perform(lufact::sycl, n);
}

