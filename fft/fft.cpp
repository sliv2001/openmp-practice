//============================================================================
// Name        : fft.cpp
// Author      : Ivan Sladkov
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <complex>
#include "omp.h"
#include <cmath>

using namespace std;

#ifndef _OPENMP
#error "No OpenMP"
#endif

#define PI	3.1415926535

int N, logN;
complex<double> *sample, *result;

void butterfly(complex<double> *x1, complex<double> *x2, int nW){
	complex<double> y, z;
	y = *x1+*x2;
	z = (-*x2+*x1)*exp(2*PI*nW/N*(-1i));
	*x1 = y;
	*x2 = z;
}

int getBitInvert(int idx) {
	int invertIdx=0;
	for (int bit = 0; bit < logN; bit++) {
		invertIdx = invertIdx << 1;
		invertIdx += (idx & 0x01);
		idx = idx >> 1;
	}
	return invertIdx;
}

void fft_s(){
	for (int iteration=1; iteration <= logN; iteration++){
		for (int step=0; step<(1<<(iteration-1)); step++){
			int nW=0;
			for (int i=N/(1<<(iteration-1))*step; i < N/(1<<(iteration-1))*step + (1<<(logN-iteration)); i++){
				butterfly(&sample[i], &sample[i + (1<<(logN-iteration))], nW);
				nW+=1<<(iteration-1);
			}
		}
	}

	for (int i=0; i<N; i++){
		int invertIdx = getBitInvert(i);
		result[invertIdx] = sample[i];
	}
}

void fft_p(int thds){
	for (int iteration=1; iteration <= logN; iteration++){
#pragma omp parallel for num_threads(thds)
		for (int step=0; step<(1<<(iteration-1)); step++){
			int nW=0;
#pragma omp simd
			for (int i=N/(1<<(iteration-1))*step; i < N/(1<<(iteration-1))*step + (1<<(logN-iteration)); i++){
				butterfly(&sample[i], &sample[i + (1<<(logN-iteration))], nW);
				nW+=1<<(iteration-1);
			}

		}
	}
#pragma omp parallel for num_threads(thds)
	for (int i=0; i<N; i++){
		int invertIdx = getBitInvert(i);
		result[invertIdx] = sample[i];
	}
}

void resetSample() {
	for (int i = 0; i < N; i++)
		sample[i] = rand();
}

int main(int argc, char** argv) {
	if (argc!=2)
		return -1;
	N = atoi(argv[1]);
	if (round(log2(N))!=log2(N))
		return -1;
	logN = log2(N);
	sample = (complex<double>*)malloc(N*sizeof(complex<double>));
	result = (complex<double>*)malloc(N*sizeof(complex<double>));
	for (int i=0; i<N; i++){
		sample[i]=rand();//sin(2*PI*i/N);
	}
	double start = omp_get_wtime();
	fft_s();
	cout<<"Время последовательного исполнения: \t\t\t"<<omp_get_wtime()-start<<endl;
	for (int i=2; i<=omp_get_max_threads(); i++){
		resetSample();
		start=omp_get_wtime();
		fft_p(i);
		cout<<"Время параллельного исполнения на "<<i<<" процессорах: \t"<<omp_get_wtime()-start<<endl;
	}
	for (; N>= 16384; N=N>>1, logN--){
		resetSample();
		start=omp_get_wtime();
		fft_p(omp_get_max_threads());
		cout<<"Время на "<<N<<" выборок: \t\t\t\t"<<omp_get_wtime()-start<<endl;
	}
	free(sample);
	free(result);
	return 0;
}
