#include <iostream>
#include <cassert>

#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_dfsTree(){
	GraphWrapper<AdjList> g = getKpq<AdjList>(4,4);
	auto u = vertex(2,g.getGraph());
	auto parent = dfsForest(g,u);
	std::vector<vertex_t<AdjList>> solution = {
		vertex(4,g.getGraph()),
		vertex(5,g.getGraph()),
		vertex(boost::graph_traits<AdjList>::null_vertex(),g.getGraph()),
		vertex(6,g.getGraph()),
		vertex(2,g.getGraph()),
		vertex(0,g.getGraph()),
		vertex(1,g.getGraph()),
		vertex(3,g.getGraph())
	};
	for(size_t i=0; i<parent.size();i++){
		ASSERT(parent[i] == solution[i]);
	}
}

auto test_dfsForest(){
	GraphWrapper<AdjList> g(8);
	add_edge(0,1,g.getGraph());
	add_edge(0,2,g.getGraph());
	add_edge(1,3,g.getGraph());
	add_edge(1,4,g.getGraph());
	add_edge(5,6,g.getGraph());
	add_edge(6,7,g.getGraph());

	auto u = vertex(0,g.getGraph());
	auto parent = dfsForest(g,u);
	std::vector<vertex_t<AdjList>> solution = {
		vertex(boost::graph_traits<AdjList>::null_vertex(),g.getGraph()),
		vertex(0,g.getGraph()),
		vertex(0,g.getGraph()),
		vertex(1,g.getGraph()),
		vertex(1,g.getGraph()),
		vertex(boost::graph_traits<AdjList>::null_vertex(),g.getGraph()),
		vertex(5,g.getGraph()),
		vertex(6,g.getGraph())
	};
	for(size_t i=0; i<parent.size();i++){
		ASSERT(parent[i] == solution[i]);
	}
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_dfsTree();
	test_dfsForest();
}
