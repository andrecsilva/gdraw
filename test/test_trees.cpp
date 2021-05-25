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

auto test_findcycle(){
	//simple cycle
	auto g = IndexedGraph{genCycle<AdjList>(5)};

	auto cycle = findCycle(g);

	ASSERT(cycle.value().size()==5);
}

auto test_findcycle2(){
	//disconnected tree
	auto g = IndexedGraph{AdjList(6)};
	g.addEdge(0,1);
	g.addEdge(0,2);

	g.addEdge(3,4);
	g.addEdge(4,5);
	g.addEdge(3,5);

	auto cycle = findCycle(g);

	ASSERT(cycle);

}

auto test_findcycle3(){
	//disconnected graph with a cycle
	auto g = IndexedGraph{AdjList(7)};
	g.addEdge(0,1);
	g.addEdge(1,2);

	g.addEdge(3,4);
	g.addEdge(4,5);
	g.addEdge(5,6);
	g.addEdge(6,4);

	auto cycle = findCycle(g);

	ASSERT(cycle.value().size()==3);

}

auto test_fundamentalcycle(){
	auto g = IndexedGraph{AdjList(7)};

	g.addEdge(0,1);
	g.addEdge(0,2);

	g.addEdge(1,3);
	g.addEdge(1,4);

	g.addEdge(2,5);
	g.addEdge(2,6);

	auto e = g.addEdge(3,5);
	g.addEdge(4,2);

	auto bfs_tree = bfsTree(g,0);

	auto cycle = fundamentalCycle(g,bfs_tree,e);

	ASSERT(cycle.size()==5);
}


int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_findcycle();
	test_findcycle2();
	test_findcycle3();
	test_fundamentalcycle();
}
