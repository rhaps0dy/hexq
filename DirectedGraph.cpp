#include <utility>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <cassert>

#include "DirectedGraph.hpp"

using namespace std;

void tarjan(unsigned n, int &index, int &n_connected_components, vector<bool> &I,
			vector<int> &L, vector<int> &S, vector<int> &D,
			const vector<vector<int> > &V) {
    D[n] = L[n] = index++;
    S.push_back(n);
    I[n] = true;
    for (unsigned i = 0; i < V[n].size(); ++i) {
		if (D[V[n][i]] < 0) {
			tarjan(V[n][i], index, n_connected_components, I, L, S, D, V);
			L[n] = min(L[n], L[V[n][i]]);
		}
		else if (I[V[n][i]])
			L[n] = min(L[n], D[V[n][i]]);
    }
    if (D[n] == L[n]) {
		n_connected_components++;
		// todos los nodos eliminados de S pertenecen al mismo scc
		while (S[S.size() - 1] != n) {
			I[S.back()] = false;
			S.pop_back();
		}
		I[n] = false;
		S.pop_back();
	}
}

int DirectedGraph::scc(vector<int> &assignment) const {
	int index=0, n_scc=0;
	vector<bool> I(adj_list.size(), false);
	assignment = vector<int>(adj_list.size());
	vector<int> S, D(adj_list.size(), -1);
    for (unsigned n = 0; n < adj_list.size(); ++n)
		if (D[n] < 0)
			tarjan(n, index, n_scc, I, assignment, S, D, adj_list);
	return n_scc;
}

unique_ptr<DirectedGraph> DirectedGraph::assignment_merge(
	const vector<int> &assignment, int n_components) const {
	vector<set<int> > adj_set(n_components);
	// avoid repeating edges when we create the adjacency list
	int sa, sb;
	for(size_t i=0; i<adj_list.size(); i++) {
		for(size_t j=0; j<adj_list[i].size(); j++)
			if((sa=assignment[i]) != (sb=assignment[adj_list[i][j]]))
				adj_set[sa].insert(sb);
	}
	auto g = unique_ptr<DirectedGraph>(new DirectedGraph());
	g->adj_list.clear();
	g->adj_list.resize(n_components);
	for(size_t i=0; i<n_components; i++) {
		for(auto jt=adj_set[i].begin(); jt != adj_set[i].end(); jt++)
			g->adj_list[i].push_back(*jt);
	}
	return move(g);
}

void DirectedGraph::merge_nodes(int n1, int n2) {
	for(size_t i=0; i<adj_list[n2].size(); i++) {
		adj_list[n1].push_back(adj_list[n2][i]);
		edge_labels[n1].push_back(edge_labels[n2][i]);
	}
	size_t last_node = adj_list.size()-1;
	adj_list[n2] = adj_list[last_node];
	edge_labels[n2] = edge_labels[last_node];
	for(size_t i=0; i<adj_list.size(); i++)
		for(size_t j=0; j<adj_list.size(); j++) {
			if(adj_list[i][j] == n2)
				adj_list[i][j] = n1;
			else if(adj_list[i][j] == last_node)
				adj_list[i][j] = n2;
		}
}

void DirectedGraph::save_dot(string filename) const {
	ofstream f(filename);
	f << "strict digraph {";
	for(size_t i=0; i<adj_list.size(); i++)
		f << "\n" << i << ";";
	for(size_t i=0; i<adj_list.size(); i++)
		for(size_t j=0; j<adj_list[i].size(); j++) {
			f << '\n' << i << " -> " << adj_list[i][j];
			if(edge_labels.size() != 0)
				f << " [label = " << edge_labels[i][j] << "];";
			else
				f << ';';
		}
	f << "\n}\n";
}

void DirectedGraph::save(string filename) const {
	ofstream f(filename);
	f << adj_list.size() << endl;
	for(size_t i=0; i<adj_list.size(); i++) {
		f << adj_list[i].size();
		for(size_t j=0; j<adj_list[i].size(); j++)
			f << ' ' << adj_list[i][j];
		f << endl;
	}
	for(size_t i=0; i<edge_labels.size(); i++) {
		for(size_t j=0; j<edge_labels[i].size(); j++)
			f << ' ' << edge_labels[i][j];
		f << endl;
	}
}

void DirectedGraph::load(string filename) {
	adj_list.clear();
	edge_labels.clear();
	ifstream f(filename);
	int n;
	f >> n;
	adj_list.resize(n);
	edge_labels.resize(n);
	for(size_t i=0; i<adj_list.size(); i++) {
		f >> n;
		adj_list[i].resize(n);
		for(size_t j=0; j<adj_list[i].size(); j++)
			f >> adj_list[i][j];
	}
	for(size_t i=0; i<edge_labels.size(); i++) {
		edge_labels[i].resize(adj_list[i].size());
		for(size_t j=0; j<edge_labels[i].size(); j++)
			if(!(f >> edge_labels[i][j]))
				goto no_labels;
	}
	return;
no_labels:
	edge_labels.clear();
}

#define CHECK(a) if(!(a)) return false
bool DirectedGraph::operator==(const DirectedGraph &b) const {
	CHECK(adj_list.size() == b.adj_list.size());
	CHECK(edge_labels.size() == b.edge_labels.size());
	for(size_t i=0; i<adj_list.size(); i++) {
		CHECK(adj_list[i].size() == b.adj_list[i].size());
		for(size_t j=0; j<adj_list[i].size(); j++) {
			CHECK(adj_list[i][j] == b.adj_list[i][j]);
			if(edge_labels.size() != 0)
				CHECK(edge_labels[i][j] == b.edge_labels[i][j]);
		}
	}
	return true;
}

// small testing of the functions here
#ifdef TEST_DIRECTEDGRAPH

#define TEST(condition, name) do {				  \
	if(condition) cout << name << ": successful\n"; \
	else cout << name << ": unsuccessful\n";		  \
} while(0)

int main() {
	DirectedGraph dg;
	vector<vector<int> > &a = dg.adj_list;
	vector<vector<string> > &b = dg.edge_labels;
	a.resize(10);
	b.resize(10);
	a[0].push_back(1);
	b[0].push_back("pa");
	a[0].push_back(3);
	b[0].push_back("ci");
	a[0].push_back(5);
	b[0].push_back("mu");
	a[1].push_back(9);
	b[1].push_back("so");
	a[2].push_back(5);
	b[2].push_back("mu");
	a[3].push_back(7);
	b[3].push_back("ze");
	a[3].push_back(9);
	b[3].push_back("so");
	a[6].push_back(4);
	b[6].push_back("vo");
	a[9].push_back(3);
	b[9].push_back("ci");
	a[9].push_back(2);
	b[9].push_back("re");
	a[9].push_back(6);
	b[9].push_back("xa");

	dg.save("/tmp/test.graph");
	DirectedGraph dgg;
	dgg.load("/tmp/test.graph");
	TEST(dg == dgg, "Save and load graph");
	dg.save_dot("/tmp/graph.dot");
	return 0;
}

#endif
