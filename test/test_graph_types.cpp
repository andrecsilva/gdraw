#include <iostream>
#include <cassert>

#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_edgelist(){
	auto g = IndexedGraph{getKpq<AdjList>(3,3)};

	auto el = EdgeList(g);
	auto h = el.edge(8);

	el.removeEdge(el.edge(0));
	ASSERT(el.g.index(h) == 0);

	auto e = el.addEdge(1,0);
	auto f = el.addEdge(1,2);

	ASSERT(el.g.index(e) == 8);
	ASSERT(el.g.index(f) == 9);
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_edgelist();
}
