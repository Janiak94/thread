#include <stdio.h>


// d is the degree of the polynomial, return exact roots
void find_roots_exact(int d, double *roots_exact){
	roots_exact[0]=1;
}

int main(void){
	double roots_exact[1];
	find_roots_exact(1, roots_exact);
	printf("%d\n", roots_exact[0]);
	return 0;
}
