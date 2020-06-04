#include <iostream>
#include <vector>
#include <map>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_dfs.hpp>

template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

//Type for embedding scheme
//Maybe edge_t instead of std::pair?
//template <typename Graph>
using embedding_t = typename std::vector<std::vector<std::pair<size_t,int>>>;

//Reads embeddings from Myrvold and Campbell's program
embedding_t readEmbedding(){
	size_t ngraph = 0;
	size_t nvertices = 0;

	std::cin >> ngraph;
	std::cin >> nvertices;

	embedding_t embedding {nvertices};
	
	for(size_t i=0; i< nvertices; i++){
		size_t degree=0;
		std::cin >> degree;	

		embedding.at(i) = std::vector<std::pair<size_t,int>>{degree};
		for (size_t j =0; j<degree; j++){
			size_t vertex=0;
			int signal=0;
			std::cin >> vertex >> signal;
			embedding.at(i).at(j) = std::make_pair(vertex,signal);
		}
			
	}
	return embedding;
}

template <typename Graph>
struct DFSTree : public boost::dfs_visitor<>{

	//Don't like this, but it seems the visitor is copied into the dfs procedure.
	std::vector<edge_t<Graph>>* treeEdges;

	DFSTree() : treeEdges{} {};
	DFSTree(std::vector<edge_t<Graph>>& ref) {treeEdges = &ref;};

	void tree_edge(edge_t<Graph> e, const Graph& g){
		treeEdges->push_back(e);
		//TODO crude...
		num_edges(g);
		std::cout << "Inserting: " << e << std::endl;
	}

};

//finds the dfs forest from a given graph
template <typename Graph>
std::vector<edge_t<Graph>> dfsTree(const Graph& g, const vertex_t<Graph>& root){

	std::vector<edge_t<Graph>> treeEdges;
	
	DFSTree<Graph> vis{treeEdges};

	//TODO maybe an iterator property map? the grpah already has edge_index_t as internal...
	std::map<edge_t<Graph>, boost::default_color_type> edge_color;

	auto ecmap = make_assoc_property_map(edge_color);

	undirected_dfs(g,
			boost::root_vertex(root)
			.visitor(vis)
			.edge_color_map(ecmap));

	std::cout << "Before return... " << std::endl;

	for(auto e: treeEdges){
		std::cout<< e << " " << std::endl;
	}

	return treeEdges;

}

//returns the number of negative edges in the path from the root to the other vertices
template <typename Graph>
std::vector<vertex_t<Graph>> pathFromRoot(const Graph& g, const embedding_t& embedding, const std::vector<edge_t<Graph>> tree, const vertex_t<Graph>& root);

template <typename Graph>
void planarDoubleCover(Graph& g, const embedding_t& embedding);
