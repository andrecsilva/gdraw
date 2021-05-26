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


auto test_planarXNumber()
{
	auto g = IndexedGraph<AdjList>{gdraw::getKpq<AdjList>(3,4)};
	auto h = IndexedGraph<AdjList>{gdraw::getKpq<AdjList>(3,3)};
	auto k = IndexedGraph<AdjList>{gdraw::getKn<AdjList>(6)};
	auto l = IndexedGraph<AdjList>{gdraw::getKn<AdjList>(4)};

	ASSERT(!planarXNumber(std::move(g),1));
	ASSERT(planarXNumber(std::move(h),2));
	ASSERT(planarXNumber(std::move(k),3));
	ASSERT(planarXNumber(std::move(l),1));
}

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

auto test_smalles1sidescycle(){
	IndexedGraph<AdjList> g {gdraw::getKpq<AdjList>(3,3)};
	
	rotations_t<AdjList> rotations = {
		{edge(0,3,g.getGraph()).first,edge(0,4,g.getGraph()).first,edge(0,5,g.getGraph()).first},
		{edge(1,5,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(1,3,g.getGraph()).first},
		{edge(2,3,g.getGraph()).first,edge(2,5,g.getGraph()).first,edge(2,4,g.getGraph()).first},
		{edge(3,1,g.getGraph()).first,edge(3,2,g.getGraph()).first,edge(3,0,g.getGraph()).first},
		{edge(4,0,g.getGraph()).first,edge(4,2,g.getGraph()).first,edge(4,1,g.getGraph()).first},
		{edge(5,0,g.getGraph()).first,edge(5,2,g.getGraph()).first,edge(5,1,g.getGraph()).first}
	};

	auto edgei_map = get(boost::edge_index, g.getGraph());
	std::vector<int> esignals(num_edges(g.getGraph()),1);

	esignals[boost::get(edgei_map,edge(1,4,g.getGraph()).first)] = -1;
	esignals[boost::get(edgei_map,edge(2,5,g.getGraph()).first)] = -1;

	auto eg = EmbeddedGraph<AdjList>(std::move(g),std::move(rotations),std::move(esignals));

	auto cycle = smallest1SidedCycle(eg);

	//for(auto&& e : cycle)
	//	std::cout << e << ' ';
	//std::cout << std::endl;

	ASSERT(cycle.size()==4);

}


int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_isPlanar();
	test_maximal();

	test_planarXNumber();
	test_doubleCover();
	test_doublePlanarCover();
	test_largestfacialcycle();
	test_smalles1sidescycle();
}
