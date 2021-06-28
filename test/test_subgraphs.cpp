#include <iostream>
#include <functional>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>
#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>

using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using namespace gdraw;

auto test_createSubgraph(){

	auto g = IndexedGraph{getKn<AdjList>(4)};

	auto dfs_tree = dfsForest(g,0);

	auto te = treeEdges(dfs_tree);
	std::vector<edge_t<AdjList>> edges (te.begin(),te.end());

	auto subg = createSubgraph(g,edges);

	ASSERT(subg.numVertices()==4 && subg.numEdges()==3);
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_createSubgraph();

}
