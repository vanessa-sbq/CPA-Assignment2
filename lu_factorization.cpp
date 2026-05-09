#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace std;

double *A, *L, *U;

const std::string powercap_path = "/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj";

/**
 * Reads energy consumption in microjoules from the specified file path.
 * @return The energy consumption in microjoules.
 */
long long read_energy_uj() {
    std::ifstream f(powercap_path); // directory path where the energy_uj file is located.
	if (f.fail())
		return 0;

	// This part is commented for the benchmark on FEUP's PCs
	// if (f.fail()) { 
	// 	fprintf(stderr, "Failed to open %s: %s\n", powercap_path.c_str(), strerror(errno));
	// 	if (errno == ENOENT) {
	// 		fprintf(stderr, "Is powercap installed?\n");
	// 	} else if (errno == EACCES) {
	// 		fprintf(stderr, "Try running with sudo to get energy information\n");
	// 		return 0;
	// 	}
	// 	exit(1);
	// }
	
    long long val; f >> val;
    return val;
}


/**
 * Prints the contents of a matrix.
 * @param M Pointer to the matrix data
 * @param n Number of rows/columns in the matrix
 * @param name Name of the matrix to display
 */
void print_matrix(double *M, int n, const char* name) {
    cout << name << " = [" << endl;
    for (int i = 0; i < n; i++) {
        cout << "  ";
        for (int j = 0; j < n; j++) {
            cout << M[i*n + j] << " ";
        }
        cout << endl;
    }
    cout << "]" << endl;
}


/**
 * Starts or resets the matrices for LU factorization.
 * @param n Number of rows/columns of matrix A
 */
void startOrResetMatrices(int n) {
	A = (double *)malloc((n * n) * sizeof(double));
	L = (double *)malloc((n * n) * sizeof(double));
	U = (double *)malloc((n * n) * sizeof(double));

	int i, j;

	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			A[i * n + j] = (i == j) ? (double)n : 1.0; // FIXME: Not sure if this should be initialized like this
			L[i * n + j] = (i == j) ? 1.0 : 0.0;
			U[i * n + j] = 0.0;
		}
	}

    // TODO: Remove (DEBUG)
    print_matrix(A, n, "A");
    print_matrix(L, n, "L");
    print_matrix(U, n, "U");
}


/**
 * Frees the matrices.
 */
void freeMatrices() {
	free(A);
	free(L);
	free(U);
}


/**
 * Displays the first row of L and U (up to 10 elements).
 * @param n Number of columns in the matrices
 */
void show_result_matrix(int n){
	int j;
	cout << "L[0,*]: ";
	for (j = 0; j < std::min(10, n); j++) {
		cout << L[j] << " ";
	}
	cout << endl;
	cout << "U[0,*]: ";
	for (j = 0; j < std::min(10, n); j++) {
		cout << U[j] << " ";
	}
	cout << endl;
}


/**
 * Displays the execution time, GFlop/s, and energy consumption for LU factorization.
 * @param start The start time of the operation.
 * @param end The end time of the operation.
 * @param n Number of rows/columns of matrix A
 * @param e_before Energy consumption before the operation (in microjoules)
 * @param e_after Energy consumption after the operation (in microjoules)
 */
void display_measurements(double start, double end, int n, double e_before, double e_after){
	double executionTime = (double)(end - start);
	printf("Time: %g seconds\n", executionTime);

	// 2/3 * n^3 flops per factorization
	double gflops = (2.0 / 3.0) * n * n * n / (executionTime * 1e9);
	printf("GFlop/s: %g\n", gflops);

	double joules = (e_after - e_before) / 1e6;
	double watts = joules / executionTime;
	printf("Joules: %.6f\n", joules);
	printf("Watts: %.6f\n", watts);
}


/**
 * TODO: Implement sequential LU factorization.
 */
void lu_fact_sequential(int n) {	
	startOrResetMatrices(n);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	display_measurements(start, end, n, e_before, e_after);
	show_result_matrix(n);

	freeMatrices();
}


/**
 * TODO: Implement block-oriented LU factorization.
 */
void lu_fact_block(int n, int block_size) {	
	(void)block_size;
	startOrResetMatrices(n);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	display_measurements(start, end, n, e_before, e_after);
	show_result_matrix(n);

	freeMatrices();
}


/**
 * TODO: Implement shared-memory LU factorization using OpenMP.
 */
void lu_fact_omp(int n, int num_threads) {	
	startOrResetMatrices(n);
	
	omp_set_num_threads(num_threads);

	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	display_measurements(start, end, n, e_before, e_after);
	show_result_matrix(n);

	freeMatrices();
}


/**
 * TODO: Implement SYCL LU factorization.
 */
void lu_fact_sycl(int n) {	
	startOrResetMatrices(n);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	display_measurements(start, end, n, e_before, e_after);
	show_result_matrix(n);

	freeMatrices();
}


/**
 * Main function to execute the LU factorization based on user input.
 */
int main () {
	int n = 0;
	int nt = 1;
	int block_size = 64;
	int op = 1;

	do {
		cout << endl;
		cout << "1. Sequential LU" << endl;
		cout << "2. Block LU" << endl;
		cout << "3. OpenMP LU" << endl;
		cout << "4. SYCL LU" << endl;
		cout << "5. Exit" << endl;
		cin >> op;
		if (op == 5) break;

		printf("Matrix size n (n x n) ? ");
		cin >> n;
		if (n <= 0) {
			cout << "Invalid n." << endl;
			continue;
		}

		switch (op) {
			case 1:
				lu_fact_sequential(n);
				break;
			case 2:
				printf("Block size ? ");
				cin >> block_size;
				if (block_size <= 0) {
					cout << "Invalid block size." << endl;
					break;
				}
				lu_fact_block(n, block_size);
				break;
			case 3:
				printf("Number of threads? ");
				cin >> nt;
				if (nt <= 0) {
					cout << "Invalid thread count." << endl;
					break;
				}
				lu_fact_omp(n, nt);
				break;
			case 4:
				lu_fact_sycl(n);
				break;
			default:
				cout << "Unknown option." << endl;
				break;
		}
	} while (op != 0);
}
