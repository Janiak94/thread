#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>

// store the values of the power d in decimal and binary (as a vector)
// these will have to be declared external if we choose keep this external
int *d_binary;
int d_binary_length;
int d;

// will it be difficult to access x from here, multiple times?
void new_point(complex double *x, int d){
	// x_temp will be the value as the power raises
	// f is the power value
	complex double x_temp = *x, f;
	for(size_t rep = 0; rep < d_binary[0]*(d_binary_length-1); ++rep){
		x_temp*=x_temp;
	}
	f = x_temp;
	for(size_t idx = 1; idx < d_binary_length; ++idx){
		complex double x_temp = *x;
		for(size_t rep = 0; rep < d_binary[idx]*(d_binary_length-idx-1); ++rep){
			// can we utilize both FPU:s here?
			x_temp *= x_temp;
		}
		if(d_binary[idx] != 0){
			f *= x_temp;
		}
	}
	// generates the next point
	(*x) = (*x) - (f-1)/(d*f/(*x));
}

void binary_rep_recur(int d, int idx);

// this will only be done once, so recursion is no problem
void binary_rep(int d){
	if(d == 0){
		exit(-1);
	}
	d_binary[d_binary_length-1]=d%2;
	// ?
	if(d_binary_length > 0){
		binary_rep_recur(d/2, d_binary_length-2);
	}
}
void binary_rep_recur(int d, int idx){
	d_binary[idx]=d%2;
	if(idx > 0){
		binary_rep_recur(d/2, idx-1);
	}
}

// here is an example of the functions
int main(void){
	d = 3;
	d_binary_length = (int)(log(d)/log(2) + 1);
	d_binary = (int*) malloc((int)(sizeof(int) * d_binary_length));
	binary_rep(d);

	complex double x = 4.0;
	for(int i = 0; i < 1e8; ++i)
		new_point(&x, d);
	printf("Next point: %f\n", creal(x));

	printf("d in binary: ");
	for(int ix = 0; ix < d_binary_length; ++ix){
		printf("%d", d_binary[ix]);
	}
	printf("\n");

	free(d_binary);
	return 0;
}
