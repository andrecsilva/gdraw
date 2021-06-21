#include <iostream>
#include <cassert>


#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>
#include <gdraw/planar_graphs.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_isPlanar(){
	auto k5 = IndexedGraph<AdjList>{getKn<AdjList>(5)};

	auto v = planeEmbedding(k5);

	ASSERT(std::holds_alternative<NonPlanarGraph<AdjList>>(v));

	//std::cout << "K5 is not planar - edges of the Kuratowski subgraph: " << std::endl;

	boost::remove_edge(0,1,k5.getGraph());

	v = planeEmbedding(std::move(k5));

	ASSERT(std::holds_alternative<PlanarGraph<AdjList>>(v));

	//printEmbedding(std::get<0>(v));

}

auto test_largestfacialcycle(){
	auto g = IndexedGraph{AdjList(6)};

	//add_edge(0,1,g.getGraph());
	//
	g.addEdge(0,5);

	//add_edge(0,3,g.getGraph());
	//
	g.addEdge(1,2);
	g.addEdge(1,5);

	//add_edge(2,3,g.getGraph());
	//
	g.addEdge(2,4);
	g.addEdge(3,4);
	g.addEdge(4,5);

	g.addEdge(6,1);
	g.addEdge(6,0);
	g.addEdge(6,5);

	g.addEdge(7,2);
	g.addEdge(7,3);
	g.addEdge(7,4);

	g.addEdge(8,0);
	g.addEdge(8,5);
	g.addEdge(8,9);

	g.addEdge(9,3);
	g.addEdge(9,4);

	g.addEdge(2,5);

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));
	auto facial_cycle = findLargestFacialCycle(pg);
	
	ASSERT(facial_cycle.size() ==8)
}

auto test_maximal(){
	size_t n = 3;
	auto g = IndexedGraph{genPath<AdjList>(n)};

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));

	pg = makeMaximal(std::move(pg));

	ASSERT(num_edges(pg.getGraph()) == 3*(n+1) - 6);
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_isPlanar();
	test_maximal();
	test_largestfacialcycle();
}
