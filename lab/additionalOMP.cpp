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
	double **a, **b;
	if (allocate(&a, ISIZE, JSIZE))
		return -1;
	if (allocate(&b, ISIZE, JSIZE))
		return -3;
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j] = 10*i+j;
			b[i][j] = 10*i+j;
		}
	}

	double start=omp_get_wtime();
#pragma omp parallel for
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j]=sin(0.04*b[(i+3)>(ISIZE-1)?ISIZE-1:i+3][j-4<0?0:j-4]);
		}
	}
	cout<<"Time elapsed for parallel: \t"<<omp_get_wtime()-start<<endl;

	FILE *ff = fopen("resultOMP_1b.txt", "w");
	if (ff==NULL)
		return -2;
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			fprintf(ff, "%f", a[i][j]);
		}
	}
	fclose(ff);

	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j] = 10*i+j;
		}
	}

	start=omp_get_wtime();
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j]=sin(0.04*b[(i+3)>(ISIZE-1)?ISIZE-1:i+3][j-4<0?0:j-4]);
		}
	}
	cout<<"Time elapsed for sequential: \t"<<omp_get_wtime()-start<<endl;

	ff = fopen("resultOMP_1b_sequential.txt", "w");
	if (ff==NULL)
		return -2;
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE; j++){
			fprintf(ff, "%f", a[i][j]);
		}
	}
	fclose(ff);
	return 0;
}
