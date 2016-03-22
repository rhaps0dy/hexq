#ifndef _DirectedGraph_hpp_
#define _DirectedGraph_hpp_

#include <memory>
#include <vector>
#include <string>
#include <utility>

using namespace std;

struct DirectedGraph {
	vector<vector<int> > adj_list;
	// edge_labels may be an empty vector. In that case, the graph has no labels.
	// Otherwise it should have the same dimensions as adj_list
	vector<vector<string> > edge_labels;

	// Complexity |V| + |E|
	int scc(vector<int> &assignment) const;

	// edge labels and parallel edges are completely erased when merging an assignment
	unique_ptr<DirectedGraph> assignment_merge(const vector<int> &assignment,
		int n_components) const;

	// may create parallel edges
	void merge_nodes(int n1, int n2);

	void save_dot(string filename) const;
	void save(string filename) const;
	void load(string filename);
	bool operator==(const DirectedGraph &b) const;
};

#endif // _DirectedGraph_hpp_
