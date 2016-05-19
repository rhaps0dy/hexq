#include "explained_assert.hpp"
#include <fstream>
#include <iostream>

typedef double Reward;

using namespace std;
using namespace hexq;

int main(int argc, char **argv) {
	vector<Reward> Q;
	for(int i=1; i<argc; i++) {
		ifstream q(argv[i]);
		sparse_vector_load(q, Q);
		q.close();
		double a=0, b=0;
		for(size_t j=0; j<Q.size(); j++) {
			if(Q[j] != 0.0) {
				a += Q[j];
				b += 1;
			}
		}
		cout << argv[i] <<": " << a/b << endl;
	}
	return 0;
}
