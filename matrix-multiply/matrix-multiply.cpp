/*
 * matrix-multiply.cpp
 *
 *  Created on: 3 окт. 2022 г.
 *      Author: ivans
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include <omp.h>

#define DEFAULT_SCALE	2000
#define SEED			1
#define MAXVAL			INT_MAX

struct Matrix {
	int scale;
	int **field;
};
typedef struct Matrix Matrix;

void err(int res, std::string strErr) {
	std::cout << "error " << res << " " << strErr << std::endl;
	exit(res);
}

int allocMatrix(Matrix *m, int scale) {
	m->scale = scale;
	m->field = (int**) malloc(sizeof(int*) * m->scale);
	if (m->field == NULL)
		return 1;
	for (int i = 0; i < m->scale; i++) {
		m->field[i] = (int*) malloc(sizeof(int) * m->scale);
		if (m->field[i] == NULL)
			return 1;
	}
	return 0;
}

void fillMatrix(Matrix m) {
	for (int i = 0; i < m.scale; i++) {
		for (int j = 0; j < m.scale; j++)
			m.field[i][j] = rand() % MAXVAL;
	}
}

void fillMatrixNum(Matrix m, int n) {
	for (int i = 0; i < m.scale; i++) {
		for (int j = 0; j < m.scale; j++)
			m.field[i][j] = n;
	}
}

void printMatrix(Matrix m) {
	for (int i = 0; i < m.scale; i++) {
		for (int j = 0; j < m.scale; j++)
			std::cout << m.field[i][j] << " ";
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

void freeMatrix(Matrix m) {
	for (int i = 0; i < m.scale; i++) {
		free(m.field[i]);
	}
	free(m.field);
}

int multiplySequential(Matrix a, Matrix b, Matrix c) {
	if (a.scale != b.scale || b.scale != c.scale)
		err(-1, "Wrong sizes");
	int scale = a.scale;

	for (int k = 0; k < scale; k++) {
		for (int i = 0; i < scale; i++) {
			int r = a.field[i][k];
			for (int j = 0; j < scale; j++) {
				c.field[i][j] += r * b.field[k][j];
			}
		}
	}
	return 0;
}

int multiplyParallel(Matrix a, Matrix b, Matrix c) {
	if (a.scale != b.scale || b.scale != c.scale)
		err(-1, "Wrong sizes");
	int scale = a.scale;
#pragma omp parallel for
	for (int k = 0; k < scale; k++) {
		for (int i = 0; i < scale; i++) {
			int r = a.field[i][k];
			for (int j = 0; j < scale; j++) {
				c.field[i][j] += r * b.field[k][j];
			}
		}
	}
	return 0;
}

int multiplySimd(Matrix a, Matrix b, Matrix c) {
	if (a.scale != b.scale || b.scale != c.scale)
		err(-1, "Wrong sizes");
	int scale = a.scale;
	for (int k = 0; k < scale; k++) {
		for (int i = 0; i < scale; i++) {
			int r = a.field[i][k];
#pragma omp simd
			for (int j = 0; j < scale; j++) {
				c.field[i][j] += r * b.field[k][j];
			}
		}
	}
	return 0;
}

int main(int argc, char **argv) {
#ifndef _OPENMP
#error No openmp
#endif
	int scale = DEFAULT_SCALE;
	if (argc == 2)
		scale = atoi(argv[1]);
	srand(SEED);

	Matrix a, b, c;
	if (allocMatrix(&a, scale) || allocMatrix(&b, scale)
			|| allocMatrix(&c, scale))
		err(-1, "Wrong memory allocation");

	fillMatrix(a);
	fillMatrix(b);
	fillMatrixNum(c, 0);
	double start = omp_get_wtime();
	multiplySequential(a, b, c);
	std::cout << "Последовательное вычисление: " << (omp_get_wtime() - start)
			<< std::endl;

	fillMatrixNum(c, 0);
	start = omp_get_wtime();
	multiplyParallel(a, b, c);
	std::cout << "Параллельное вычисление: " << (omp_get_wtime() - start)
			<< std::endl;

	fillMatrixNum(c, 0);
	start = omp_get_wtime();
	multiplySimd(a, b, c);
	std::cout << "SIMD вычисление: " << (omp_get_wtime() - start) << std::endl;

	fillMatrixNum(c, 0);
	start = omp_get_wtime();
	multiplySequential(a, b, c);
	std::cout << "Последовательное вычисление: " << (omp_get_wtime() - start)
			<< std::endl;

	freeMatrix(a);
	freeMatrix(b);
	freeMatrix(c);
}

