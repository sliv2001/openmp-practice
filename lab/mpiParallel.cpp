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

int main(int argc, char** argv){
	MPI_Init(&argc, &argv);

	int rank, procs, ball;
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double **a;
	int istart=(ISIZE/(procs-1))*rank, iend = (ISIZE/(procs-1))*(rank+1);
	/* Распараллеливание по блокам, а не по строкам позволяет поэкономить
	 * выделяемую память
	 */
	if (rank+1==procs){
		istart = (ISIZE/(procs-1))*(procs-1);
		iend = ISIZE;
	}
	if (allocate(&a, iend-istart, JSIZE))
		return -1;
	for (int i=0; i<iend-istart; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j] = 10*(istart+i)+j;
		}
	}

	double start=MPI_Wtime();
	for (int i=0; i<iend-istart; i++){
		for (int j=0; j<JSIZE; j++){
			a[i][j]=sin(2*a[i][j]);
		}
	}
	cout<<"Time elapsed: \t"<<MPI_Wtime()-start;

	/*Последовательно с 0 до procs-1 отправка процессом rank в файл */
	if (rank!=0){
		MPI_Recv(&ball, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	FILE *ff = fopen("resultMPI.txt", rank?"a":"w");
	if (ff==NULL)
		return -2;
	for (int i=0; i<iend-istart; i++){
		for (int j=0; j<JSIZE; j++){
			fprintf(ff, "%f", a[i][j]);
		}
	}
	fclose(ff);
	if (rank+1<procs)
		MPI_Send(&ball, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);

	MPI_Finalize();
	return 0;
}
