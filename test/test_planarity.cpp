#include <iostream>
#include <cassert>


#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>
#include <gdraw/xnumber.hpp>
#include <gdraw/pplane.hpp>
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
	auto k5 = GraphWrapper<AdjList>{getKn<AdjList>(5)};

	auto v = planeEmbedding(k5);

	ASSERT(std::holds_alternative<NonPlanarGraph<AdjList>>(v));

	//std::cout << "K5 is not planar - edges of the Kuratowski subgraph: " << std::endl;

	boost::remove_edge(0,1,k5.getGraph());

	v = planeEmbedding(std::move(k5));

	ASSERT(std::holds_alternative<PlanarGraph<AdjList>>(v));

	//printEmbedding(std::get<0>(v));

}

auto test_maximal(){
	size_t n = 3;
	auto g = GraphWrapper{genPath<AdjList>(n)};

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));

	pg = makeMaximal(std::move(pg));

	printGraph(pg);

	ASSERT(num_edges(pg.getGraph()) == 3*n - 6);
}


auto test_planarXNumber()
{
	auto g = GraphWrapper<AdjList>{gdraw::getKpq<AdjList>(3,4)};
	auto h = GraphWrapper<AdjList>{gdraw::getKpq<AdjList>(3,3)};
	auto k = GraphWrapper<AdjList>{gdraw::getKn<AdjList>(6)};
	auto l = GraphWrapper<AdjList>{gdraw::getKn<AdjList>(4)};

	ASSERT(!planarXNumber(std::move(g),1));
	ASSERT(planarXNumber(std::move(h),2));
	ASSERT(planarXNumber(std::move(k),3));
	ASSERT(planarXNumber(std::move(l),1));
}

auto test_doubleCover()
{
	auto g = GraphWrapper<AdjList>{gdraw::getKpq<AdjList>(3,3)};
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

	auto g = GraphWrapper<AdjList>{gdraw::getKn<AdjList>(6)};

	auto result = findDoublePlanarCover(g);

	ASSERT(result);
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_isPlanar();
	test_maximal();

	test_planarXNumber();
	test_doubleCover();
	test_doublePlanarCover();
}
