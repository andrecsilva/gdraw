#include <iostream>
#include <cassert>


#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/embedded_graphs.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;


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

auto test_allFacialWalks(){
	auto g = IndexedGraph{genCycle<AdjList>(5)};

	g.addEdge(5,0);

	//printGraph(g);

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(std::move(g)));

	auto all_walks = allFacialWalks(pg);

	//for(auto&& w : all_walks){
	//	for(auto&& e : w)
	//		std::cout << e << ' ';
	//	std::cout << std::endl;
	//}

	ASSERT(all_walks.size() ==2);
	ASSERT(all_walks[0].size() ==7);
	ASSERT(all_walks[1].size() ==5);


	//for(auto&& e : walk)
	//	std::cout << e << ' ';
	//std::cout << std::endl;

	//for(auto&& e : walk2)
	//	std::cout << e << ' ';
	//std::cout << std::endl;
	
}

auto test_allFacialWalks2(){
	//One-sided cycle
	auto g = IndexedGraph{genCycle<AdjList>(5)};

	//printGraph(g);

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(std::move(g)));

	pg.edge_signals[pg.index(pg.edge(0,4).value())] = -1;

	auto all_walks = allFacialWalks(pg);

	//for(auto&& e : walk)
	//	std::cout << e << ' ';
	//std::cout << std::endl;
	
	ASSERT(all_walks.size()==1);
	ASSERT(all_walks[0].size()==10);

}

auto test_allFacialWalks3(){
	IndexedGraph<AdjList> g {gdraw::getKpq<AdjList>(3,3)};
	
	rotations_t<AdjList> rotations = {
		{edge(0,3,g.getGraph()).first,edge(0,5,g.getGraph()).first,edge(0,4,g.getGraph()).first},
		{edge(1,3,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(1,5,g.getGraph()).first},
		{edge(2,3,g.getGraph()).first,edge(2,4,g.getGraph()).first,edge(2,5,g.getGraph()).first},
		{edge(3,0,g.getGraph()).first,edge(3,2,g.getGraph()).first,edge(3,1,g.getGraph()).first},
		{edge(4,0,g.getGraph()).first,edge(4,1,g.getGraph()).first,edge(4,2,g.getGraph()).first},
		{edge(5,0,g.getGraph()).first,edge(5,2,g.getGraph()).first,edge(5,1,g.getGraph()).first}
	};

	auto edgei_map = get(boost::edge_index, g.getGraph());
	std::vector<int> esignals(num_edges(g.getGraph()),1);

	esignals[boost::get(edgei_map,edge(1,3,g.getGraph()).first)] = -1;
	esignals[boost::get(edgei_map,edge(2,5,g.getGraph()).first)] = -1;

	auto eg = EmbeddedGraph<AdjList>(std::move(g),std::move(rotations),std::move(esignals));

	auto all_walks = allFacialWalks(eg);

	//for(auto&& w : all_walks){
	//	for(auto&& e : w)
	//		std::cout << e << ' ';
	//	std::cout << std::endl;
	//}
	ASSERT(all_walks.size()==4)
}

auto test_allFacialWalks4(){
	auto g = IndexedGraph<AdjList>{getKpq<AdjList>(3,3)};

	rotations_t<AdjList> rotations = {
		{edge(0,3,g.getGraph()).first,edge(0,5,g.getGraph()).first,edge(0,4,g.getGraph()).first},
		{edge(1,3,g.getGraph()).first,edge(1,5,g.getGraph()).first,edge(1,4,g.getGraph()).first},
		{edge(2,3,g.getGraph()).first,edge(2,5,g.getGraph()).first,edge(2,4,g.getGraph()).first},
		{edge(3,0,g.getGraph()).first,edge(3,2,g.getGraph()).first,edge(3,1,g.getGraph()).first},
		{edge(4,0,g.getGraph()).first,edge(4,2,g.getGraph()).first,edge(4,1,g.getGraph()).first},
		{edge(5,0,g.getGraph()).first,edge(5,2,g.getGraph()).first,edge(5,1,g.getGraph()).first}
	};

	auto edgei_map = get(boost::edge_index, g.getGraph());
	std::vector<int> esignals(num_edges(g.getGraph()),1);

	esignals[boost::get(edgei_map,edge(0,4,g.getGraph()).first)] = -1;
	esignals[boost::get(edgei_map,edge(2,3,g.getGraph()).first)] = -1;
	esignals[boost::get(edgei_map,edge(1,5,g.getGraph()).first)] = -1;

	auto eg = EmbeddedGraph<AdjList>(std::move(g),std::move(rotations),std::move(esignals));

	auto all_walks = allFacialWalks(eg);

	ASSERT(all_walks.size()==4);
}


int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_smalles1sidescycle();
	test_allFacialWalks();
	test_allFacialWalks2();
	test_allFacialWalks3();
	test_allFacialWalks4();
}
