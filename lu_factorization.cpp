#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace std;

double *L, *U, *phc;

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
 * Starts or resets the matrices.
 * @param m_ar Number of rows/columns of matrix A
 * @param m_br Number of rows/columns of matrix B
 */
void startOrResetMatrices(int m_ar, int m_br) {
	L = (double *)malloc((m_ar * m_ar) * sizeof(double));
	U = (double *)malloc((m_ar * m_ar) * sizeof(double));
	phc = (double *)calloc((m_ar * m_ar), sizeof(double));

	int i, j;

	for(i=0; i<m_ar; i++)
		for(j=0; j<m_ar; j++)
			L[i*m_ar + j] = (double)1.0;

	for(i=0; i<m_br; i++)
		for(j=0; j<m_br; j++)
			U[i*m_br + j] = (double)(i+1);
}


/**
 * Frees the matrices.
 */
void freeMatrices() {
	free(L);
	free(U);
	free(phc);
}


/**
 * Displays the first row of the result matrix (up to 10 elements).
 * @param m_r Number of columns in the result matrix
 */
void show_result_matrix(int m_r){
	int i, j;
	cout << "Result matrix: " << endl;
	for(i=0; i<1; i++) {
		for(j=0; j<min(10,m_r); j++)
			cout << phc[j] << " ";
	}

	cout << endl;
}


/**
 * TODO:
 */
void lu_fact_sequential(int m_ar, int m_br) {	
	startOrResetMatrices(m_ar, m_br);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	show_result_matrix(m_br);

	freeMatrices();
}


/**
 * TODO:
 */
void lu_fact_block(int m_ar, int m_br) {	
	startOrResetMatrices(m_ar, m_br);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	show_result_matrix(m_br);

	freeMatrices();
}


/**
 * TODO:
 */
void lu_fact_omp(int m_ar, int m_br) {	
	startOrResetMatrices(m_ar, m_br);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	show_result_matrix(m_br);

	freeMatrices();
}


/**
 * TODO:
 */
void lu_fact_sycl(int m_ar, int m_br) {	
	startOrResetMatrices(m_ar, m_br);
	
	auto e_before = read_energy_uj();
	double start = omp_get_wtime(); // Get start time
	
	// TODO:
	
	double end = omp_get_wtime(); // Get end time
	auto e_after = read_energy_uj();

	show_result_matrix(m_br);

	freeMatrices();
}








int main () {
	int lin, col, nt=1;
	int op;

	op=1;
	do {
		cout << endl << "1. Multiplication" << endl;
		cout << "2. Line Multiplication" << endl;
		cout << "3. Parallel Line Multiplication" << endl;
		cout << "4. Parallel Line Multiplication with SIMD" << endl;
		cin >> op;
		if (op == 0) break;
		printf("Dimensions: lins cols ? ");
   		cin >> lin >> col;

		switch (op) {
			case 1: 
				OnMult(lin, col);
				break;
			case 2:
				OnMultLine(lin, col);
				break;
            case 3:
                printf("Number of threads? ");
                cin >> nt;
                OnMultLineParallel(lin, col, nt);
                break;
            case 4:
                printf("Number of threads? ");
                cin >> nt;
                OnMultLineParallelSIMD(lin, col, nt);
                break;
		}
	} while (op != 0);
}
