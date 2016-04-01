#include <utility>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>

#include "explained_assert.hpp"
#include "directed_graph.hpp"

using namespace std;

void Tarjan(unsigned n, int &index, int &n_connected_components, vector<bool> &I,
			Assignment &L, vector<int> &S, vector<int> &D,
			const vector<vector<int> > &V) {
    D[n] = L[n] = index++;
    S.push_back(n);
    I[n] = true;
    for (int i = 0; i < V[n].size(); ++i) {
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
    for (int n = 0; n < adj_list.size(); ++n)
		if (D[n] < 0)
			Tarjan(n, index, n_scc, I, assignment, S, D, adj_list);
	return n_scc;
}

unique_ptr<DirectedGraph> DirectedGraph::MergeByAssignment(
	const Assignment &assignment, int n_components) const {
	vector<set<int> > adj_set(n_components);
	// avoid repeating edges when we create the adjacency list
	int sa, sb;
	for(int i=0; i<adj_list.size(); i++) {
		for(int j=0; j<adj_list[i].size(); j++)
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

/*void DirectedGraph::merge_nodes(int n1, int n2) {
	for(int i=0; i<adj_list[n2].size(); i++) {
		adj_list[n1].push_back(adj_list[n2][i]);
		edge_labels[n1].push_back(edge_labels[n2][i]);
	}
	int last_node = adj_list.size()-1;
	adj_list[n2] = adj_list[last_node];
	edge_labels[n2] = edge_labels[last_node];
	for(int i=0; i<adj_list.size(); i++)
		for(int j=0; j<adj_list.size(); j++) {
			if(adj_list[i][j] == n2)
				adj_list[i][j] = n1;
			else if(adj_list[i][j] == last_node)
				adj_list[i][j] = n2;
		}
		}*/

void DirectedGraph::SaveDot(const string filename) const {
	ofstream f(filename);
	f << "strict digraph {";
	for(int i=0; i<adj_list.size(); i++)
		f << "\n" << i << ";";
	for(int i=0; i<adj_list.size(); i++)
		for(int j=0; j<adj_list[i].size(); j++) {
			f << '\n' << i << " -> " << adj_list[i][j];
			if(edge_labels.size() != 0)
				f << " [label = " << edge_labels[i][j] << "];";
			else
				f << ';';
		}
	f << "\n}\n";
}

void DirectedGraph::Save(const string filename) const {
	ofstream f(filename);
	f << adj_list.size() << endl;
	for(int i=0; i<adj_list.size(); i++) {
		f << adj_list[i].size();
		for(int j=0; j<adj_list[i].size(); j++)
			f << ' ' << adj_list[i][j];
		f << endl;
	}
	for(int i=0; i<edge_labels.size(); i++) {
		for(int j=0; j<edge_labels[i].size(); j++)
			f << ' ' << edge_labels[i][j];
		f << endl;
	}
}

void DirectedGraph::Load(const string filename) {
	adj_list.clear();
	edge_labels.clear();
	ifstream f(filename);
	int n;
	f >> n;
	adj_list.resize(n);
	edge_labels.resize(n);
	for(int i=0; i<adj_list.size(); i++) {
		f >> n;
		adj_list[i].resize(n);
		for(int j=0; j<adj_list[i].size(); j++)
			f >> adj_list[i][j];
	}
	for(int i=0; i<edge_labels.size(); i++) {
		edge_labels[i].resize(adj_list[i].size());
		for(int j=0; j<edge_labels[i].size(); j++)
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
	for(int i=0; i<adj_list.size(); i++) {
		CHECK(adj_list[i].size() == b.adj_list[i].size());
		for(int j=0; j<adj_list[i].size(); j++) {
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
	for(int i=0; i<adj_list.size(); i++)
		for(int j=0; j<adj_list[i].size(); j++) {
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

	dg.Save("/tmp/test.graph");
	DirectedGraph dgg;
	dgg.Load("/tmp/test.graph");
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
