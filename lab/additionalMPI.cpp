#include <iostream>
#include <cstdlib>
#include <cmath>
#include <mpi.h>
#include <assert.h>

#define ISIZE 5000
#define JSIZE 5000

using namespace std;

int allocate(double ***a, int i, int j) {
	*a = (double**) malloc(sizeof(double*) * i);
	if (*a == NULL)
		return -1;
	for (int iter = 0; iter < i; iter++) {
		(*a)[iter] = (double*) malloc(sizeof(double) * j);
		if (**a == NULL)
			return -1;
	}
	return 0;
}

void save1(double **a){
	FILE *ff = fopen("resultMPI_1a.txt", "w");
	if (ff == NULL)
		exit(-2);
	for (int i=0; i<ISIZE; i++)
		for (int j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f", a[i][j]);
		}
	fclose(ff);
}

void save2(double **a){
	FILE *ff = fopen("resultMPI_1a_seq.txt", "w");
	if (ff == NULL)
		exit(-2);
	for (int i=0; i<ISIZE; i++)
		for (int j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f", a[i][j]);
		}
	fclose(ff);
}

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int rank, procs;
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double **a;
	if (allocate(&a, ISIZE, JSIZE))
		return -1;
	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) {
			a[i][j] = 10 * i + j;
		}
	}

	/*[i-1][j+1]; a[i][JSIZE] inited the same way*/

	double start = MPI_Wtime();
	MPI_Request r;
	MPI_Status s;
	int diag;
	for (diag=rank; diag<JSIZE; diag+=procs){
		int j=diag;
		for (int i=1; i<=diag; i++){
			j--;
			if (j<0||j>JSIZE||i<0||i>ISIZE){
				cout<<"shit1 "<<i<<" "<<j<<endl;
				exit(10);
			}
			a[i][j]=sin(2*a[i-1][j+1]);
		}
	}
	for (++diag; diag<JSIZE+ISIZE; diag+=procs){
		int j=JSIZE-1;
		//if (!rank) cout<<"shit1 "<<diag<<endl;
		for (int i=diag-ISIZE+1; i<ISIZE; i++){
			j--;

			a[i][j]=sin(2*a[i-1][j+1]);
		}
	}
	cout<<"Time elapsed for parallel: \t"<<MPI_Wtime()-start<<endl;
	MPI_Barrier(MPI_COMM_WORLD);


	if (rank){
		double *buff = (double*)malloc(sizeof(double)*JSIZE);
		for (diag=rank; diag<JSIZE; diag+=procs){
			int j=diag;
			for (int i=0; i<=diag; i++){
				buff[j]=a[i][j];
				j--;
			}
			MPI_Send(buff, diag+1, MPI_DOUBLE, 0, diag, MPI_COMM_WORLD);
		}
		for (; diag<JSIZE+ISIZE-1; diag+=procs){
			int j=JSIZE-1;
			for (int i=diag-ISIZE+1; i<ISIZE; i++){

				buff[j]=a[i][j];
				j--;
			}
			int k=0;
			for (int i=0; i<2*ISIZE-diag-1; i++){
				buff[k]=buff[diag+1-ISIZE+i];
				k++;
			}
			MPI_Send(buff, 2*ISIZE-diag-1, MPI_DOUBLE, 0, diag, MPI_COMM_WORLD);
		}
	}
	else{
		double *buff = (double*)malloc(sizeof(double)*JSIZE);
		for (diag=0; diag<JSIZE; diag+=1){
			if (diag%procs==0)
				continue;
			MPI_Recv(buff, diag+1, MPI_DOUBLE, MPI_ANY_SOURCE, diag, MPI_COMM_WORLD, &s);
			int j=diag;
			for (int i=0; i<=diag; i++){
				a[i][j]=buff[j];
				j--;
			}
		}
		for (; diag<JSIZE+ISIZE-1; diag+=1){
			if (diag%procs==0)
				continue;
			MPI_Recv(buff, 2*ISIZE-diag-1, MPI_DOUBLE, MPI_ANY_SOURCE, diag, MPI_COMM_WORLD, &s);
			int k=ISIZE-1;
			for (int i=2*ISIZE-diag-2; i>=0; i--){
				buff[k]=buff[i];
				k--;
			}
			int j=JSIZE-1;
			for (int i=diag-ISIZE+1; i<ISIZE; i++){

				a[i][j]=buff[j];
				j--;
			}
		}
	}
	save1(a);



	if (rank){
				MPI_Finalize();
		return 0;
	}
	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE; j++) {
			a[i][j] = 10 * i + j;
		}
	}
		start=MPI_Wtime();
	for (int i = 1; i < ISIZE; i += 1) {
		for (int j = 0; j < JSIZE-1; j++) {
			if (i > 0) {
				a[i][j] = sin(2 * a[i - 1][j + 1]);
			}
		}
	}
		cout<<"Time elapsed for sequential: \t"<<MPI_Wtime()-start<<endl;
	save1(a);

		MPI_Finalize();
	return 0;
}
