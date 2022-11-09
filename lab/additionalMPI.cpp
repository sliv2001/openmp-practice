#include <iostream>
#include <cstdlib>
#include <cmath>
#include <mpi.h>

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

void save(double **a, int N, bool rewrite) {
	FILE *ff = fopen("resultMPI.txt", rewrite?"w":"a");
	if (ff == NULL)
		exit(-2);
	for (int j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f", a[N][j]);
	}

	fclose(ff);
}

int main(int argc, char** argv){
	MPI_Init(&argc, &argv);

	int rank, procs, ball;
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double **a;
	if (allocate(&a, ISIZE, JSIZE+1))
		return -1;
	for (int i=0; i<ISIZE; i++){
		for (int j=0; j<JSIZE+1; j++){
			a[i][j] = 10*i+j;
		}
	}

	/*[i-1][j+1]; a[i][JSIZE] inited the same way*/

	double start=MPI_Wtime();
	MPI_Request r;
	for (int i=rank; i<ISIZE; i+=procs){
		for (int j=0; j<JSIZE; j++){
			if (i>0){
				MPI_Recv(&a[i-1][j+1], 1, MPI_DOUBLE, rank?rank-1:procs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				a[i][j]=sin(2*a[i-1][j+1]);
			}

			if (i<ISIZE-1)
				MPI_Send(&a[i][j], 1, MPI_DOUBLE, rank+1==procs?0:rank+1, 0, MPI_COMM_WORLD);
		}
	}
	cout<<"Time elapsed: \t"<<MPI_Wtime()-start;

	MPI_Barrier(MPI_COMM_WORLD);
	ball=0;
	if (!rank){
		save(a, ball, 1);
		ball++;
		MPI_Send(&ball, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	for (int i=rank; i<ISIZE; i+=procs){
		MPI_Recv(&ball, 1, MPI_INT, rank?rank-1:procs-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		save(a, ball, 0);
		ball++;
		MPI_Send(&ball, 1, MPI_INT, rank+1==procs?0:rank+1, 0, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Finalize();
	return 0;
}
