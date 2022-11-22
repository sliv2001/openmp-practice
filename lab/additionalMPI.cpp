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

void saveSeq(double **a, int N){
	FILE *ff = fopen("resultMPI_1a_seq.txt", "w");
	if (ff == NULL)
		exit(-2);
	for (int i=0; i<ISIZE; i++)
		for (int j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f", a[i][j]);
		}
	fclose(ff);
}

void save(double **a, int N){
	FILE *ff = fopen("resultMPI_1a.txt", "w");
	if (ff == NULL)
		exit(-2);
	for (int i=0; i<ISIZE; i++)
		for (int j = 0; j < JSIZE; j++) {
			fprintf(ff, "%f", a[i][j]);
		}
	fclose(ff);
}

void save(double **a, int N, bool rewrite) {
	if (N>-1) return;
	FILE *ff = fopen("resultMPI_1a.txt", rewrite ? "w" : "a");
	if (ff == NULL)
		exit(-2);
	for (int j = 0; j < JSIZE; j++) {
		fprintf(ff, "%f", a[N][j]);
	}

	fclose(ff);
}

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);

	int rank, procs, ball;
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double **a;
	if (allocate(&a, ISIZE, JSIZE + 1))
		return -1;
	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE + 1; j++) {
			a[i][j] = 10 * i + j;
		}
	}
	
	/*[i-1][j+1]; a[i][JSIZE] inited the same way*/

	double start = MPI_Wtime();
	MPI_Request r;
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
	for (; diag<JSIZE+ISIZE; diag+=procs){
		int j=JSIZE-1;
		//if (!rank) cout<<"shit1 "<<diag<<endl;
		for (int i=diag-ISIZE+1; i<ISIZE; i++){
			j--;
			
			a[i][j]=sin(2*a[i-1][j+1]);
		}
	}

	cout << "Time elapsed: \t" << MPI_Wtime() - start;

	MPI_Barrier(MPI_COMM_WORLD);
	ball = 0;
	if (!rank) {
		save(a, ball, 1);
		ball++;
		MPI_Send(&ball, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	for (int i = rank; i < ISIZE; i += procs) {
		MPI_Recv(&ball, 1, MPI_INT, rank ? rank - 1 : procs - 1, 0,
				MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (ball >= ISIZE)
			break;
		save(a, ball, 0);
		ball++;
		MPI_Send(&ball, 1, MPI_INT, rank + 1 == procs ? 0 : rank + 1, 0,
				MPI_COMM_WORLD);
		if (ball >= ISIZE - procs)
			break;
	}

	if (rank) {
		MPI_Finalize();
		return 0;
	}
	
	for (int i = 0; i < ISIZE; i++) {
		for (int j = 0; j < JSIZE + 1; j++) {
			a[i][j] = 10 * i + j;
		}
	}
	start = MPI_Wtime();
	for (int i = 0; i < ISIZE; i += 1) {
		for (int j = 0; j < JSIZE; j++) {
			if (i > 0) {
				a[i][j] = sin(2 * a[i - 1][j + 1]);
			}
		}
	}
	cout <<endl<< "Time elapsed for sequential: \t" << MPI_Wtime() - start;
	saveSeq(a, 0);
	save(a, 0);
	MPI_Finalize();
	return 0;
}
