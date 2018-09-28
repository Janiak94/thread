#include <stdio.h>
#include <complex.h>
#include <math.h>

// d is the degree of the polynomial, return exact roots
void find_roots_exact(int d, double *roots_exact){
	roots_exact[0]=1;
}

int main(void){
	
	int d;
	d = 5;
	// DEFINE CORRECT ROOTS
	double complex roots_exact[d];
	for (int i = 0; i<d; ++i) {
		roots_exact[i] = cos(2*M_PI*i/d) + sin(2*M_PI*i/d) * I;
	}
	for (int i = 0; i<d; i++) {
		printf("Root %d = %f + %f i\n", i, creal(roots_exact[i]), cimag(roots_exact[i]));
	}

}
