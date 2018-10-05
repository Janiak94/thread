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
	
	double complex value = 1;
	for (int i = 0; i<d-1; ++i) {
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


void find_root(double complex x_k, int * elem_ptr) {
	// TODO: Check condition every other iteration?
	int root_idx; 
	int i = 0;
	while ( (root_idx = close_to_root(x_k) ) == -1  ) {
		x_k = iterate(x_k);
		//printf("x_%d = %f + %f i, \n", i, creal(x_k), cimag(x_k));
		++i;
	}
	*elem_ptr = root_idx;
	*(elem_ptr + 1) = i;
}

void root_colors(int d, int * colors){
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

	//Define data matrix containing both number of interations and root index
	int * entr = (int*) malloc(sizeof(int)* number_of_points*number_of_points*2);
	int ** row_ptr = (int**) malloc(sizeof(int*) * number_of_points);
	for(size_t i = 0; i < number_of_points; ++i){
		row_ptr[i] = entr + i*2*number_of_points;
	}

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
			printf("Root %d = %f + %f i\n", 
				i, creal(roots_exact[i]), cimag(roots_exact[i]));
		}
	}
	// Find root index and number of iterations for all points
	double complex start_point;
	for(size_t i = 0; i < number_of_points; ++i){
		for( size_t j = 0; j < number_of_points; ++j){
			start_point = i/((double)number_of_points-1)*4 -2 +
				 (j/((double)number_of_points-1)*4 - 2)*I;
			find_root(start_point, &row_ptr[i][2*j]); 
		 }
	}
	/*if (debug)
		printf("Test function_f. x = %f + %f i, f(x) = %f + %f i\n", 
			creal(test_number), cimag(test_number),
			creal(test_answer), cimag(test_answer));
	*/
	FILE *g_file, *c_file;
	
	char c_file_name[50];
	sprintf(c_file_name, "newton_attractors_x%d.ppm", d);
	char g_file_name[50];
	sprintf(g_file_name,"newton_convergence_x%d.ppm", d);
	
	c_file = fopen(c_file_name, "w");
	g_file = fopen(g_file_name, "w");
	
	int colors[6][3]={{255,0,0},{0,255,0},{0,0,255},{255,255,0},{0,255,255},{255,0,255}};

	fprintf(c_file, "P3\n%d %d\n255\n", number_of_points, number_of_points);
	fprintf(g_file, "P3\n%d %d\n255\n", number_of_points, number_of_points);
	int g_temp;
	int c_temp[3];
	for(size_t i = 0; i < number_of_points; ++i){
		for(size_t j = 0; j < number_of_points*2; j+=2){
			g_temp = row_ptr[i][j+1] > 255 ? 0 : 255 - row_ptr[i][j+1];
			c_temp[0] = colors[row_ptr[i][j]][ 0];
			c_temp[1] = colors[row_ptr[i][j]][ 1];
			c_temp[2] = colors[row_ptr[i][j]][ 2];
			fprintf(c_file, " %d %d %d\t", c_temp[0], c_temp[1], c_temp[2]); 
			fprintf(g_file, " %d %d %d\t", g_temp, g_temp, g_temp);
			if(0)
				printf("index: %d, it: %d\n", row_ptr[i][j], row_ptr[i][j+1]);
		}
		fprintf(c_file, "\n");
		fprintf(g_file, "\n");
	}
			 

	fclose(c_file);
	fclose(g_file);	

	free(entr);
	free(row_ptr);
	free(roots_exact);
	return 0;
}
