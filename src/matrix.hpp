#ifndef __MATRIX__
#define __MATRIX__

template<typename T = double>
class Matrix {
    public:
    explicit Matrix<T>(unsigned size);
    ~Matrix<T>();

    /**
     * Gets the value of the element in the i-th line and j-th column of the matrix
     * @param i line index
     * @param j column index
     * @return T value of the element in the i-th line and j-th column of the matrix
     */
    inline auto get(unsigned i, unsigned j) const -> T;

    /**
     * Sets the value of the element in the i-th line and j-th column of the matrix
     * @param i line index
     * @param j column index
     * @param value value to be set in the matrix
     */
    inline auto set(unsigned i, unsigned j, T value) -> void;

    /**
     * Prints the contents of a matrix
     * @param name Name of the matrix to be displayed
     */
    auto print(const char* name = "M") const -> void;
    
    /**
     * Previews the first elements and lines of a matrix. Shows at most 10 elements per line and at most 10 lines
     * @param name Name of the matrix to be displayed
     */
    auto preview(const char* name = "M") const -> void;

    public:
    /// Number of lines and columns of the matrix
    const unsigned size;

    private:
    T* const m;
};

/**
 * Starts or resets the matrices for LU factorization
 * @param n Number of elements of vectors b, x, y
 * @param A Matrix A
 * @param b Pointer to vector b
 * @param x Pointer to vector x
 * @param y Pointer to vector y
 */
auto startOrResetMatrices(
    unsigned n,
    Matrix<double>& A,
    double*& b,
    double*& x,
    double*& y
) -> void;

/**
 * Frees the matrices.
 * @param b Pointer to vector b
 * @param x Pointer to vector x
 * @param y Pointer to vector y
 */
auto freeMatrices(
    const double* b,
    const double* x,
    const double* y
) -> void;

/**
 * Solves a equation of type `Ax=b` using LU factorization
 * @param A Matrix A
 * @param b Vector b
 * @param x Vector x (solution)
 * @param y Vector y (auxiliary vector)
 */
auto solve(
    Matrix<double>& A,
    const double* b,
    double* x,
    double* y
) -> void;

#endif // __MATRIX__
