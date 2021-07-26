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

auto test_scc(){
	std::vector<std::vector<int>> g(8);

	//Wikipedia's example
	g[0].push_back(1);
	g[1].push_back(2);
	g[1].push_back(4);
	g[1].push_back(5);
	g[2].push_back(3);
	g[2].push_back(6);
	g[3].push_back(2);
	g[3].push_back(7);
	g[4].push_back(0);
	g[4].push_back(5);
	g[5].push_back(6);
	g[6].push_back(5);
	g[7].push_back(6);
	g[7].push_back(3);

	auto comp = stronglyConnectedComponents(g);

	//for(auto&& i :comp)
	//	std::cout << i << ' ';
	//std::cout << std::endl;

	ASSERT(comp[0]==0);
	ASSERT(comp[1]==0);
	ASSERT(comp[2]==1);
	ASSERT(comp[3]==1);
	ASSERT(comp[4]==0);
	ASSERT(comp[5]==2);
	ASSERT(comp[6]==2);
	ASSERT(comp[7]==1);
}


int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;
	test_scc();
}
