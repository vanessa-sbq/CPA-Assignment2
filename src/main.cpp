#include <iostream>
#include <omp.h>
#include "perf.hpp"
#include "lu_factorization.hpp"

/**
 * Main function to execute the LU factorization based on user input.
 */
int main () {
    int n = 0;
    int nt = 1;
    int block_size = 64;
    int op = 1;

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

        std::cout << "Matrix size n (n x n) ? " << std::endl;
        std::cout << "> ";
        std::cin >> n;
        if (n <= 0) {
            std::cout << "Invalid n." << std::endl;
            continue;
        }

        switch (op) {
            case 1:
                measure(lu_fact_sequential, n);
                break;
            case 2:
                std::cout << "Block size ? " << std::endl;
                std::cout << "> ";
                std::cin >> block_size;
                if (block_size <= 0) {
                    std::cout << "Invalid block size." << std::endl;
                    break;
                }
                measure([block_size](int n) {
                    lu_fact_block(n, block_size);
                }, n);
                break;
            case 3:
                std::cout << "Number of threads? " << std::endl;
                std::cout << "> ";
                std::cin >> nt;
                if (nt <= 0) {
                    std::cout << "Invalid thread count." << std::endl;
                    break;
                }
                measure([nt](int n) {
                    omp_set_num_threads(nt);
                    lu_fact_omp(n, nt);
                }, n);
                break;
            case 4:
                measure(lu_fact_sycl, n);
                break;
            default:
                std::cout << "Unknown option." << std::endl;
                break;
        }
    } while (op != 0);
}
