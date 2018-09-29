#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

// is it possible to optimize even further with respect to
// gantenbeins two FPU:s?
// this might not be faster since we perform more operations, we need to
// perform operations on d every time the function is called
void new_point_reduced(complex double *x, int d_in){
	complex double x_temp, f = 1;
	int d = d_in;

	// computes x^d with log2(d)? operations
	for(int i = 0; d/2 > 0 || d%2 > 0; ++i){
		x_temp = *x;
		if(d%2){
			for(int j = 0; j < i; ++j){
				x_temp *= x_temp;
			}
			f *= x_temp;
		}
		d = d/2;
	}
	*x = *x - (f-1)/(d_in*f/(*x));
}

int main(void){
	int d = 2;
	complex double x = 4.0;
	for(int i = 0; i < 1e8; ++i)
		new_point_reduced(&x, d);
	printf("Next point: %f\n", creal(x));

	return 0;
}
