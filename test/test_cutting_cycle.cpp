#include <iostream>
#include <functional>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>
#include <gdraw/generators.hpp>
#include <gdraw/embedded_graphs.hpp>

using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using namespace gdraw;

auto test_cut_1_sided_cycle(){
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

	std::vector<edge_t<AdjList>> cycle1 = {edge(0,3,eg.getGraph()).first,edge(3,1,eg.getGraph()).first,edge(1,4,eg.getGraph()).first,edge(4,0,eg.getGraph()).first};
	std::vector<edge_t<AdjList>> cycle2 = {edge(5,2,eg.getGraph()).first,edge(2,4,eg.getGraph()).first,edge(4,0,eg.getGraph()).first,edge(0,3,eg.getGraph()).first,edge(3,1,eg.getGraph()).first,edge(1,5,eg.getGraph()).first};
	cutAlongCycle(eg,cycle1);

	ASSERT(isOrientable(eg));
	auto v = planeEmbedding(eg);
	ASSERT(std::holds_alternative<PlanarGraph<AdjList>>(v));

	//auto pg = std::get<PlanarGraph<AdjList>>(v);
	//auto dg = chrobakPayneDraw(pg);
	//writeDOT(dg);

};

auto test_positivetree_1(){
	IndexedGraph<AdjList> g {gdraw::getKn<AdjList>(4)};
	auto mg = planeEmbedding(std::move(g));
	EmbeddedGraph<AdjList> pg = std::get<PlanarGraph<AdjList>>(mg);

	auto edgei_map = get(boost::edge_index, pg.getGraph());
	
	auto e = edge(0,1,pg.getGraph()).first;
	auto e_index = boost::get(edgei_map,e);
	pg.edge_signals[e_index]=-1;

	e = edge(0,2,pg.getGraph()).first;
	e_index = boost::get(edgei_map,e);
	pg.edge_signals[e_index]=-1;

	e = edge(3,1,pg.getGraph()).first;
	e_index = boost::get(edgei_map,e);
	pg.edge_signals[e_index]=-1;

	e = edge(3,2,pg.getGraph()).first;
	e_index = boost::get(edgei_map,e);
	pg.edge_signals[e_index]=-1;

	ASSERT(isOrientable(pg));
}

auto test_positivetree_2(){
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

	ASSERT(!isOrientable(eg));
}

int main(){
	test_positivetree_1();
	test_positivetree_2();
	test_cut_1_sided_cycle();
}
