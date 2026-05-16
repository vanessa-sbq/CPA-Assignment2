#include <algorithm>
#include <iostream>
#include <ostream>
#include <random>
#include "matrix.hpp"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 10.0); // TODO: Consider using a larger range

/**
 * @brief Returns a random number in range of [0.0, 10.0]
 * @return double
 */
double rnd() {
    return dis(gen);
}

template<typename T>
Matrix<T>::Matrix(unsigned size):
    size(size),
    m(new T[size * size]) {}

template<typename T>
Matrix<T>::~Matrix() {
    delete [] m;
}

template<typename T>
auto Matrix<T>::get(unsigned i, unsigned j) const -> T {
    return this->m[i * this->size + j];   
}

template<typename T>
auto Matrix<T>::set(unsigned i, unsigned j, T value) -> void {
    this->m[i * this->size + j] = value;
}

template<typename T>
auto Matrix<T>::print(const char* name) const -> void {
    std::cout << name << " = [" << std::endl;
    for (unsigned i = 0; i < this->size; i++) {
        std::cout << "  ";
        for (unsigned j = 0; j < this->size; j++) {
            std::cout << this->get(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

template<typename T>
auto Matrix<T>::preview(const char* name) const -> void {
    for (unsigned i = 0 ; i < std::min(10u, this->size); i++) {
        std::cout << name << "[" << i << ",*]: ";
        for (unsigned j = 0; j < std::min(10u, this->size); j++) {
            std::cout << this->get(i, j) << " ";
        }
        std::cout << std::endl;
    }
}

template class Matrix<double>;

auto startOrResetMatrices(
    unsigned n,
    Matrix<>& A,
    double*& b,
    double*& x,
    double*& y
) -> void {
    b = new double[n];
    x = new double[n];
    y = new double[n];

    for (unsigned i = 0; i < n; i++) {
        for (unsigned j = 0; j < n; j++) {
            A.set(i, j, rnd());
        }
        b[i] = rnd();
    }

    // TODO: Remove (DEBUG)
    // A.print("A");
    // std::cout << "b[*]: ";
    // for (unsigned i = 0; i < n; i++) {
    //     std::cout << b[i] << " ";
    // }
    // std::cout << std::endl;
}

auto freeMatrices(
    const double* b,
    const double* x,
    const double* y
) -> void {
    delete[] b;
    delete[] x;
    delete[] y;
}

auto solve(
    Matrix<>& A,
    const double* const b,
    double* const x,
    double* const y
) -> void {
    // TODO: Rewrite this function
    (void) A;
    (void) b;
    (void) x;
    (void) y;
    // Forward Substitution:
    // for (int i = 0; i < n; i++) {
    //     double sum = 0.0;
    //     for (int j = 0; j < i; j++)
    //         sum += L[i * n + j] * y[j];
    //     y[i] = b[i] - sum; // L[i*n + i] == 1.0 for Doolittle
    // }

    // // Backward Substitution:
    // for (int i = n - 1; i >= 0; i--) {
    //     double sum = 0.0;
    //     for (int j = i + 1; j < n; j++)
    //         sum += U[i * n + j] * x[j];
    //     x[i] = (y[i] - sum) / U[i * n + i];
    // }
}
