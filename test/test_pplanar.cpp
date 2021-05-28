#include <iostream>
#include <cassert>


#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>
#include <gdraw/pplane.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_doubleCover()
{
	auto g = IndexedGraph<AdjList>{gdraw::getKpq<AdjList>(3,3)};
	add_edge(0,1,g.getGraph());

	std::vector<edge_t<AdjList>> xedges = {edge(0,3,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(2,5,g.getGraph()).first};

	auto n = num_vertices(g.getGraph());
	auto m = num_edges(g.getGraph());

	auto dc = doubleCover(std::move(g),std::move(xedges));

	auto nc = num_vertices(dc.getGraph());
	auto mc = num_edges(dc.getGraph());

	ASSERT(nc == 2*n);
	ASSERT(mc == 2*m);
}

auto test_doublePlanarCover()
{

	auto g = IndexedGraph<AdjList>{gdraw::getKn<AdjList>(6)};

	auto result = findDoublePlanarCover(std::move(g));

	ASSERT(result);
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_doubleCover();
	test_doublePlanarCover();
}
