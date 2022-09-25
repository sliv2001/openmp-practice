/*
 * star_sudoku.cpp
 *
 *  Created on: 25 сент. 2022 г.
 *      Author: ivans
 */


#include <omp.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <cmath>
#include <ctime>

#define MULTIPLIER 100
#define NUM_OF_THREADS omp_get_num_threads()

using namespace std;

uint8_t** Field; //Костыль
int Done=0;

void err(int res, string error) {
	cout << error << endl;
	exit(res);
}

uint8_t** allocAll(int scale) {
	uint8_t** field = (uint8_t**) (malloc(scale * sizeof(uint8_t*)));
	for (int i = 0; i < scale; i++) {
		field[i] = (uint8_t*) (malloc(scale));
		if (field[i] == NULL)
			err(1, "Allocation error");
	}
	return field;
}

int getFieldScale(string path) {
	ifstream fin;
	string firstLine;
	int res=0;
	fin.open(path);
	if (!fin.is_open())
		err(-2, "Couldnot open the file");
	getline(fin, firstLine);
	if ((firstLine.length()) == 0) {
		fin.close();
		err(-1, "Couldnot get line");
	}
	fin.close();
	for (unsigned int i=0; i<firstLine.length(); i++){
		if (firstLine[i]=='/')
			res++;
	}
	return res+1;
}

void print(int scale, uint8_t** field){
	for (int i=0; i<scale; i++){
		for (int j=0; j<scale; j++){
			cout<<to_string(field[i][j])<<'\t';
		}
		cout<<endl<<endl;
	}
}

int fillField(string path, int scale, uint8_t **field) {
	ifstream fin;
	string firstLine;
	fin.open(path);
	if (!fin.is_open())
		err(-2, "Couldnot open the file");

	for (int i = 0; i < scale; i++) {
		for (int j = 0; j < scale; j++) {
			int tmp;
			char emp;
			fin >> tmp;
			field[i][j] = tmp;
			fin.get(emp);
			if (emp == '\n') {
				if (j < scale - 1)
					err(-4, "Formatting issue");
			} else if (emp != '/')
				err(-3, "Formatting issue");
		}
	}

	fin.close();
	return 0;
}

int getNthEmpty(int n, int scale, uint8_t** field, int* I, int* J){
	int i, j, done=0, remain=n;
	for (i=0; i<scale; i++){
		for (j=0; j<scale; j++){
			if (field[i][j]==0){
				remain--;
				if (remain==0){
					done=1;
					break;
				}
			}
		}
		if (done==1)
			break;
	}
	*I=i;
	*J=j;
	return done;
}

int getNumPossibles(int scale, uint8_t** field, int i, int j){
	int res=0;
	for (int k=1; k<=scale; k++){
		int b=1;
		for (int n=0; n<scale; n++){
			if (field[i][n]==k && n!=j){
				b=0;
				break;
			}
			if (field[n][j]==k && n!=i){
				b=0;
				break;
			}
		}

		int semiScale=floor(sqrt(scale));
		int left = (j==0?0:j-(j%semiScale));
		int top = (i==0?0:i-(i%semiScale));
		for (int a=top; a<top+semiScale; a++){
			for (int c=left; c<left+semiScale; c++){
				if (field[a][c]==k&& a!=i && c!=j){
					b=0;
					break;
				}
			}
		}
		res+=b;
	}
	return res;
}

int getNumTasks(int scale, uint8_t **field) {
	int i, j, numTasks = 1, procs = omp_get_num_threads(), n = 1;
	do {
		if (!getNthEmpty(n, scale, field, &i, &j))
			err(-1, "");

		if (!(numTasks *= getNumPossibles(scale, field, i, j)))
			err(-1, "");

		n++;
	} while (numTasks < MULTIPLIER*procs);
	return numTasks;
}

uint8_t* getPossibles(int scale, uint8_t** field, int i, int j){
	uint8_t* res = (uint8_t*)malloc(getNumPossibles(scale, field, i, j)+1);
	int iter=0;
	for (int k=1; k<=scale; k++){
		int b=1;
		for (int n=0; n<scale; n++){
			if (field[i][n]==k && n!=j){
				b=0;
				break;
			}
			if (field[n][j]==k && n!=i){
				b=0;
				break;
			}
		}

		int semiScale=floor(sqrt(scale));
		int left = (j==0?0:j-(j%semiScale));
		int top = (i==0?0:i-(i%semiScale));
		for (int a=top; a<top+semiScale; a++){
			for (int c=left; c<left+semiScale; c++){
				if (field[a][c]==k){
					b=0;
					break;
				}
			}
		}
		if (b==1){
			res[iter++]=k;
		}
	}
	res[iter]=0;
	return res;
}

int length(uint8_t* arr){
	int res=0;
	while (arr[res]){
		res++;
	}
	return res;
}

void copyMem(uint8_t** to, uint8_t** from, int scale){
	for (int i=0; i<scale; i++)
		memcpy(to[i], from[i], scale);
}

void freeAll(uint8_t** what, int scale){
	if (what!=NULL){
		for (int i=0; i<scale; i++){
			if (what[i]!=NULL)
				free(what[i]);
		}
		free(what);
	}
}

int checkError(int scale, uint8_t** field, int i, int j){
	int res=1;
	for (int n=0; n<scale; n++){
		if (field[i][n]==field[i][j] && n!=j){
			res=0;
			break;
		}
		if (field[n][j]==field[i][j] && n!=i){
			res=0;
			break;
		}
	}

	int semiScale=floor(sqrt(scale));
	int left = (j==0?0:j-(j%semiScale));
	int top = (i==0?0:i-(i%semiScale));
	for (int a=top; a<top+semiScale; a++){
		for (int c=left; c<left+semiScale; c++){
			if (field[a][c]==field[i][j] && i!=a && j!=c){
				res=0;
				break;
			}
		}
	}
	return res;
}

int solveParallel(int scale, uint8_t** field){
	int i, j;
	if (!getNthEmpty(1, scale, field, &i, &j)){
		cout<<"DONE"<<endl;
		return 1;
	}
	if (Done==1)
		return 0;
	for (int k=1; k<=scale; k++){
		field[i][j]=k;
		if (checkError(scale, field, i, j))
			if (solveParallel(scale, field))
				return 1;
	}
	field[i][j]=0;
	return 0;
}

int solve(int scale, uint8_t** field, int cumulative=1){

	uint8_t** tmp;
	int i, j;
	if (!getNthEmpty(1, scale, field, &i, &j))
		return 0;
	uint8_t* possibles=getPossibles(scale, field, i, j);
	int k=0;
	while(possibles[k]!=0){
		tmp=allocAll(scale);
		copyMem(tmp, field, scale);
		tmp[i][j]=possibles[k];
		if (cumulative<NUM_OF_THREADS){
			if (solve(scale, tmp, cumulative*length(possibles))){
				copyMem(field, tmp, scale);
				freeAll(tmp, scale);
				free(possibles);
				return 0;
			}
		}
		else{
#pragma omp task
			{
				if (Done==0){
					cout<<time(NULL)<<" task at work"<<endl;
					if (solveParallel(scale, tmp)){
						Field=allocAll(scale);
						copyMem(Field, tmp, scale);
						Done=1;
						freeAll(tmp, scale);
					}
					else {
						freeAll(tmp, scale);
					}
				}

			}
		}
		k++;
	}
	free(possibles);
	return 0;
}

int main(int argc, char **argv) {
#if _OPENMP<201201 || !defined _OPENMP
#error NoOpenmp
#endif
	string filePath = "test1.txt";
	uint8_t **field;

	int scale = getFieldScale(filePath);

	field = allocAll(scale);
	fillField(filePath, scale, field);

	print(scale, field);
	cout<<endl<<endl<<endl<<endl;

	double start=omp_get_wtime();
	solve(scale, field);
	double timeSerial = (omp_get_wtime()-start)*omp_get_wtick();
	start=omp_get_wtime();
	Done=0;
#pragma omp parallel
	{
#pragma omp single
		{
			cout<<omp_get_num_threads()<<endl;
			solve(scale, field);
		}
	}
	double timeParallel=(omp_get_wtime()-start)*omp_get_wtick();

	print(scale, Field);
	cout<<endl;
	cout<<"Время последовательной работы: \t"<<timeSerial<<endl;
	cout<<"Время параллельной работы: \t"<<timeParallel<<endl;
	cout<<"Ускорение: \t"<<timeSerial/timeParallel<<endl;
	return 0;
}

