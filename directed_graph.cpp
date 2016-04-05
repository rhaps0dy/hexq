#include <fstream>
#include <sstream>
#include <iostream>

#include "explained_assert.hpp"
#include "directed_graph.hpp"

using namespace std;

void Tarjan(unsigned n, int &index, int &n_connected_components, vector<bool> &I,
			Assignment &L, vector<int> &S, vector<int> &D,
			const vector<vector<int> > &V) {
    D[n] = L[n] = index++;
    S.push_back(n);
    I[n] = true;
    for (size_t i = 0; i < V[n].size(); ++i) {
		if (D[V[n][i]] < 0) {
			Tarjan(V[n][i], index, n_connected_components, I, L, S, D, V);
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

int DirectedGraph::StronglyConnectedComponents(Assignment &assignment) const {
	int index=0, n_scc=0;
	vector<bool> I(adj_list.size(), false);
	assignment = Assignment(adj_list.size());
	vector<int> S, D(adj_list.size(), -1);
    for (size_t n = 0; n < adj_list.size(); ++n)
		if (D[n] < 0)
			Tarjan(n, index, n_scc, I, assignment, S, D, adj_list);
	return n_scc;
}

unique_ptr<DirectedGraph> DirectedGraph::MergeByAssignment(
	const Assignment &assignment, int n_components) const {
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
	for(int i=0; i<n_components; i++) {
		for(auto jt=adj_set[i].begin(); jt != adj_set[i].end(); jt++)
			g->adj_list[i].push_back(*jt);
	}
	return move(g);
}

void DirectedGraph::SaveDot(const string filename) const {
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

std::ostream &operator<<(std::ostream &os, DirectedGraph &dg) {
	bool t = (dg.edge_labels.size() != 0);
	os << t << ' ';
	os << dg.adj_list;
	if(t)
		os << dg.edge_labels;
	return os;
}

std::istream &operator>>(std::istream &is, DirectedGraph &dg) {
	bool t;
	is >> t;
	is >> dg.adj_list;
	if(t)
		is >> dg.edge_labels;
	return is;
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

void MergeRegionsInAssignment(Assignment &assignment, int &n_regions,
							  int n1, int n2) {
	if(n1 == n2) return;
	if(n1 > n2) swap(n1, n2);
	n_regions--;
	for(auto it=assignment.begin(); it != assignment.end(); it++) {
		if(*it == n2)
			*it = n1;
		else if(*it == n_regions)
			*it = n2;
	}
}

unique_ptr<DirectedGraph> DirectedGraph::Reverse() const {
	auto dg = unique_ptr<DirectedGraph>(new DirectedGraph());
	dg->adj_list.resize(adj_list.size());
	dg->edge_labels.resize(adj_list.size());
	for(size_t i=0; i<adj_list.size(); i++)
		for(size_t j=0; j<adj_list[i].size(); j++) {
			int n = adj_list[i][j];
			dg->adj_list[n].push_back(i);
			dg->edge_labels[n].push_back(edge_labels[i][j]);
		}
	return move(dg);
}

// small testing of the functions here
#ifdef TEST_DIRECTEDGRAPH

int main() {
	DirectedGraph dg;
	AdjacencyList &a = dg.adj_list;
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

	ofstream tfile("/tmp/test.graph");
	tfile << dg;
	DirectedGraph dgg;
	ifstream tfile2("/tmp/test.graph");
	tfile2 >> dgg;
	ASSERT(dg == dgg, "Save and load graph");
	dg.SaveDot("/tmp/graph.dot");


	AdjacencyList &aa = dgg.adj_list;
	vector<vector<string> > &bb = dgg.edge_labels;
	aa.clear();
	bb.clear();
	aa.resize(10);
	bb.resize(10);
	aa[1].push_back(0);
	bb[1].push_back("pa");
	aa[2].push_back(9);
	bb[2].push_back("re");
	aa[3].push_back(0);
	bb[3].push_back("ci");
	aa[3].push_back(9);
	bb[3].push_back("ci");
	aa[4].push_back(6);
	bb[4].push_back("vo");
	aa[5].push_back(0);
	bb[5].push_back("mu");
	aa[5].push_back(2);
	bb[5].push_back("mu");
	aa[6].push_back(9);
	bb[6].push_back("xa");
	aa[7].push_back(3);
	bb[7].push_back("ze");
	aa[9].push_back(1);
	bb[9].push_back("so");
	aa[9].push_back(3);
	bb[9].push_back("so");

	auto dggg = dg.Reverse();
	ASSERT(dgg == *dggg, "Reverse graph");
	dggg->SaveDot("/tmp/reversed.dot");
	return 0;
}

#endif
