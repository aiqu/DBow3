#include <iostream>
#include <string>
#include "DBoW3.h"

using namespace std;
using namespace DBoW3;

int main(int argc, char* argv[]){
	if(argc != 2){
		cout << "Usage: ./converter \"ORBVocaburaryFileName\"" << endl;
		return -1;
	}

	string filename(argv[1]);
	Vocabulary voca(filename);
	size_t extpos = filename.find('.');
	if(extpos != string::npos)
		voca.save(filename.substr(0,extpos));
	else
		voca.save(filename);
	return 0;
}
