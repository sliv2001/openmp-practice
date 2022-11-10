/*
 * diff.cpp
 *
 *  Created on: 10 нояб. 2022 г.
 *      Author: ivans
 */

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv){
	if (argc<3)
		return 0;
	ifstream f1, f2;
	f1.open(argv[1]);
	f2.open(argv[2]);
	if (!f1.is_open()||!f2.is_open())
		return -1;
	int i=0;
	while (!f1.eof()&&!f2.eof()){
		char c1 = f1.get();
		char c2 = f2.get();
		i++;
		if (c1!=c2){
			cout<<"No match at character "<<i<<endl;
			return 0;
		}
	}
	return 0;
	
}
