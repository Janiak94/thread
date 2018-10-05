#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

// thought: is it better to just calculate powers in a single loop?
// we only need to calculate up to d = 7, and the if-statesments might
// impare more than neccessary

// is it possible to optimize even further with respect to
// gantenbeins two FPU:s?
// this might not be faster since we perform more operations, we need to
// perform operations on d every time the function is called
complex double new_point_reduced(complex double x, int d_in){
	complex double x_temp, f = 1;
	int d = d_in;

	// computes x^d with log2(d)? operations
	for(int i = 0; d/2 > 0 || d%2 > 0; ++i){
		x_temp = x;
		if(d%2){
			for(int j = 0; j < i; ++j){
				x_temp *= x_temp;
			}
			f *= x_temp;
		}
		d = d/2;
	}
	return x - (f-1)/(d_in*f/(x));
}
complex double iterate(double complex x_k, int d) {
	//double complex x = *x_k;
	double complex value = 1;
	for (int i = 0; i<d-1; ++i) {
		value *= x_k;
	//	printf("Inside f.   Value = %f + %f\n", creal(value), cimag(value));
	}
	return x_k - (value*(x_k) - 1)/ (d*value);;
}
int main(void){
	int d = 7;
	complex double x = 2.0 + 2.0*I;
	for(int i = 0; i < 1e7; ++i)
		x=iterate(x,d);
	//	x=new_point_reduced(x, d);
	printf("Next point: %f\n", creal(x));

	return 0;
}
