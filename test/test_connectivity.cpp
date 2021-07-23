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
	std::vector<std::vector<bool>> g(8,std::vector<bool>(8,false));

	//Wikipedia's example
	g[0][1]=true;
	g[1][2]=true;
	g[1][4]=true;
	g[1][5]=true;
	g[2][3]=true;
	g[2][6]=true;
	g[3][2]=true;
	g[3][7]=true;
	g[4][0]=true;
	g[4][5]=true;
	g[5][6]=true;
	g[6][5]=true;
	g[7][6]=true;
	g[7][3]=true;


	auto comp = stronglyConnectedComponents(g);

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
