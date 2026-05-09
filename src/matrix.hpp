#ifndef __MATRIX__
#define __MATRIX__

extern double *A, *L, *U, *x, *y, *b;

/**
 * Prints the contents of a matrix.
 * @param M Pointer to the matrix data
 * @param n Number of rows/columns in the matrix
 * @param name Name of the matrix to display
 */
void print_matrix(double *M, int n, const char* name);

/**
 * Starts or resets the matrices for LU factorization.
 * @param n Number of rows/columns of matrix A
 */
void startOrResetMatrices(int n);

/**
 * Frees the matrices.
 */
void freeMatrices();

/**
 * Solves a equation of type Ax=b using LU factorization
 * @param n matrix size
 */
void solve(int n);

/**
 * Displays the first row of L and U (up to 10 elements).
 * @param n Number of columns in the matrices
 */
void show_result_matrix(int n);

#endif // __MATRIX__
