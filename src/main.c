#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define TOL 1e-6
#define MAX_VAL 1e10
// Global variables
double complex * roots_exact;
int d;

// f(x) = x^d-1
double complex iterate(double complex x) {
	
	double complex value = x;
	for (int i = 1; i<d-1; ++i) {
		value *= x;
	//	printf("Inside f.   Value = %f + %f\n", creal(value), cimag(value));
	}
	return x - (value*x - 1)/ (d*value);;
}


int close_to_root(double complex x) {
	if (creal(x) > MAX_VAL || creal(x) < -MAX_VAL || cimag(x) > MAX_VAL 
			|| cimag(x) < - MAX_VAL)
		return d;

	for (int i = 0; i<d; ++i) {
		if (  creal((x-roots_exact[i])*conj(x-roots_exact[i])) <= TOL )
			return i;
	}
	return -1;
}


int find_root(double complex x_k) {
	// TODO: Check condition every other iteration?
	int root_idx; 
	int i = 0;
	printf("Test 1\n");
	while ( (root_idx = close_to_root(x_k) ) == -1  ) {
		x_k = iterate(x_k);
		printf("x_%d = %f + %f i, \n", i, creal(x_k), cimag(x_k));
		++i;
	}	
	return root_idx;
}

int main(int argc, char *argv[]){
	
	int number_of_threads;
	int number_of_points;
	// =========================
	// READ ARGUMENTS
	// =========================
	if (argc != 4) {
		printf("ERROR, must have 3 arguments.\n");
		return -1;
	} 
	int arg1 = 0;
	int arg2 = 0;
	int arg3 = 0;
	for (int i = 1; i<4; ++i) {
		if (memcmp( "-t", argv[i], 2)== 0) {
			arg1 = 1;
			number_of_threads = (int) strtol(&argv[i][2], argv, 10);
		} else if (memcmp( "-l", argv[i], 2)== 0) {
			arg2 = 1;
			number_of_points = (int) strtol(&argv[i][2], argv, 10); 
		} else {
			// TODO: if not a number
			arg3 = 1;
			d = (int) strtol(argv[i], argv, 10);
		}
	}
	if (arg1 * arg2 * arg3 == 0) {
		printf("ERROR, all 3 arguments must be correct.\n");
		return -1;
	}
	printf("Number of points: %d\n", number_of_points);
	printf("Number of threads: %d\n", number_of_threads);
	printf("Exponential power: %d\n", d);

	int debug = 1;
	// =========================
	// DEFINE CORRECT ROOTS
	// =========================
	roots_exact = (double complex *) malloc(sizeof(double complex) * d);
	for (int i = 0; i<d; ++i) {
		roots_exact[i] = cos(2*M_PI*i/d) + sin(2*M_PI*i/d) * I;
	}
	// print roots for debugging
	if (debug) {
		for (int i = 0; i<d; i++) {
			printf("Root %d = %f + %f i\n", i, creal(roots_exact[i]), cimag(roots_exact[i]));
		}
	}

	// Test iterate function
	double complex test_number = -3 + 1*I;
	double complex test_answer = find_root(test_number);
	if (debug)
		printf("Test function_f. x = %f + %f i, f(x) = %f + %f i\n", 
			creal(test_number), cimag(test_number),
			creal(test_answer), cimag(test_answer));

	free(roots_exact);
	return 0;
}
