#include <vector>
#include <fstream>
#include <string>
#include "explained_assert.hpp"

using namespace hexq;
using namespace std;

int main(int argc, char **argv) {
	if(argc != 3)
		return 1;
	ifstream in(argv[1]);
	ofstream out(argv[2]);
	vector<double> Q;
	sparse_vector_load(in, Q);
	size_t old_q_sz = Q.size();
	Q.resize(2*old_q_sz);
	for(size_t i=0; i<old_q_sz; i++)
		Q[i+old_q_sz] = Q[i];
	sparse_vector_save(out, Q);
	in.close();
	out.close();
	return 0;
}
