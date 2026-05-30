#include <iostream>
#include <omp.h>
#include <utility>
#include <vector>
#include "hipSYCL/sycl/queue.hpp"
#include "perf.hpp"
#include "lu_fact.hpp"

// avoid naming collisions
namespace options {
inline auto sequential(unsigned n) -> void;
inline auto block(unsigned n) -> void;
inline auto omp(unsigned n) -> void;
inline auto sycl(unsigned n, void (*impl)(Matrix<double>& A, unsigned block_size, sycl::queue &qsigned)) -> void;
}

const std::vector<std::pair<const std::string, void (*)(unsigned)>> optionsvec {
    {"Sequential", options::sequential},
    {"Block", options::block},
    {"OpenMP", options::omp},
    {"SYCL (Dumb)", [](unsigned n) {options::sycl(n, lufact::sycl_dumb);}},
    {"SYCL (Basic)", [](unsigned n) {options::sycl(n, lufact::sycl_basic);}},
    {"SYCL (Block)", [](unsigned n) {options::sycl(n, lufact::sycl_block);}}, // TODO
};

/**
 * Main function to execute the LU factorization based on user input.
 */
auto main () -> int {
    unsigned n = 0;
    size_t op = 1;

    do {
        for (size_t i = 0; i < optionsvec.size(); ++i)
            std::cout << i+1 << ". " << optionsvec[i].first << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "> ";
        op = 0;
        std::cin >> op;
        if (op == 0)
            break;
        --op;
        if (op >= optionsvec.size()) {
            std::cout << "Invalid option." << std::endl;
            continue;
        }

        std::cout << "Matrix size n (n x n) ?" << std::endl;
        std::cout << "> ";
        n = 0;
        std::cin >> n;
        if (n <= 0) {
            std::cout << "Invalid n." << std::endl;
            continue;
        }

        optionsvec[op].second(n);

    } while (std::cin);
    return 0;
}

namespace options {

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

auto sycl(unsigned n, void (*impl)(Matrix<double>& A, unsigned block_size, sycl::queue &qsigned)) -> void {
    unsigned block_size = 64;
    std::cout << "Block size ? " << std::endl;
    std::cout << "> ";
    std::cin >> block_size;
    if (block_size <= 0) {
        std::cout << "Invalid block size." << std::endl;
        return;
    }
    unsigned devid = 0;
    std::cout << "Select device" << std::endl;
    const auto devices = sycl::device::get_devices();
    unsigned i = 1;
    for (const auto &device : devices) {
        std::cout << i << ". " << device.get_info<sycl::info::device::name>() << std::endl;
        ++i;
    }
    std::cout << "> ";
    std::cin >> devid;
    if (devid < 1 || devid > devices.size()) {
        std::cout << "Invalid option." << std::endl;
        return;
    }

    sycl::queue q {
        devices[devid-1],
        sycl::property::queue::in_order(),
    };
    std::cout << "Will use " << q.get_device().get_info<sycl::info::device::name>() << std::endl;

    perform([block_size, &q, impl](Matrix<>& A) {
        impl(A, block_size, q);
    }, n, &q);

    q.wait_and_throw();
}

}
