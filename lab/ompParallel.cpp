#include <iostream>
#include <cstdlib>
#include <cmath>
#include <omp.h>

#define ISIZE 5000
#define JSIZE 5000

using namespace std;

int allocate(double ***a, int i, int j){
	*a=(double**)malloc(sizeof(double*)*i);
	if (*a==NULL)
		return -1;
	for (int iter=0; iter<i; iter++){
		(*a)[iter]=(double*)malloc(sizeof(double)*j);
		if (**a==NULL)
			return -1;
	}
	return 0;
}

int main(int argc, char** argv){
	double **a;
	if (allocate(&a, ISIZE, JSIZE))
		return -1;
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j] = 10*i+j;
		}
	}

	double start=omp_get_wtime();
#pragma omp parallel for
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j]=sin(2*a[i][j]);
		}
	}
	cout<<"Time elapsed: \t"<<omp_get_wtime()-start;

	FILE *ff = fopen("resultOMP.txt", "w");
	if (ff==NULL)
		return -2;
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			fprintf(ff, "%f", a[i][j]);
		}
	}
	return 0;
}
