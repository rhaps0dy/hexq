#include <memory>
#include <vector>
#include <string>
#include <utility>

using namespace std;

typedef vector<int> Assignment;
typedef vector<vector<int> > AdjacencyList;

/**
 * \brief Merges two vectors into one and sorts their elements. You may
 * repeat vector in inputs and outputs, the results will still be correct.
 */
template<typename T>
void MergeVectors(std::vector<T> &out, const std::vector<T> &in1,
				  const std::vector<T> &in2) {
	std::set<T> s;
	for(auto i=in1.begin(); i!=in1.end(); i++)
		s.insert(*i);
	for(auto i=in2.begin(); i!=in2.end(); i++)
		s.insert(*i);
	out.clear();
	for(auto i=s.begin(); i!=s.end(); i++)
		out.push_back(*i);
}

/// Merges the regions of the assignment with value n1 and n2.
/**
 * \param assignment a vector indicating to which region belongs each node of a
 * graph
 * \param n_regions an integer indicating how many regions are there in
 * assignment
 * \param n1, n2 Regions to merge
 */
void MergeRegionsInAssignment(Assignment &assignment, int &n_regions, int
n1, int n2);

struct DirectedGraph {
	/// Adjacency list of the graph
	AdjacencyList adj_list;
	/**
	 * DirectedGraph::edge_labels may be an empty vector. In that case, the
	 * graph has no labels. Otherwise it should have the same dimensions as
	 * DirectedGraph::adj_list
	 */
	vector<vector<string> > edge_labels;

	/// Tarjan's algorithm for SCC, complexity |V| + |E|
	int StronglyConnectedComponents(Assignment &assignment) const;

	// edge labels and parallel edges are completely erased when merging an assignment
	unique_ptr<DirectedGraph> MergeByAssignment(const Assignment &assignment,
		int n_components) const;

	/// Creates a graph with all the edge directions reversed
	unique_ptr<DirectedGraph> Reverse() const;

	/// Saves a .dot file with the graph's data.
	/** This .dot file can be then be
	 * displayed using the tools in http://www.graphviz.org/. For example:
	 * \code{.sh}
	 * dot -Tpng file.dot > image.png
	 * neato -Tpdf file.dot > image.pdf
	 * \endcode
	 */
	void SaveDot(string filename) const;
	/// Saves a file that can be read back with DirectedGraph::Load
	void Save(string filename) const;
	/// Loads a file that was written with DirectedGraph::Save
	void Load(string filename);
	/**
	 * Two DirectedGraphs are equal when both their DirectedGraph::adj_list and
	 * DirectedGraph::edge_labels are
	 * equal.
	 */
	bool operator==(const DirectedGraph &b) const;
};
