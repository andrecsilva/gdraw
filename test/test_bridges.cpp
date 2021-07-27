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

auto test_bridgeOverlap(){
	auto g = IndexedGraph{genCycle<AdjList>(6)};

	std::vector<edge_t<AdjList>> facial_cycle = 
	{g.edge(0,1).value(),
	g.edge(1,2).value(),
	g.edge(2,3).value(),
	g.edge(3,4).value(),
	g.edge(4,5).value(),
	g.edge(5,0).value()
	};

	std::vector<bool> b1_attachments = {false,true,false,true,false,true};
	auto b2_attachments = b1_attachments;

	auto overlap = bridgeOverlap(g,b1_attachments,b2_attachments,facial_cycle);

	//for(auto&& v : overlap.value())
	//	std::cout << v << ' ';
	//std::cout << std::endl;

	ASSERT(overlap.size()==3);
}

auto test_bridgeOverlap2(){
	auto g = IndexedGraph{genCycle<AdjList>(6)};

	std::vector<edge_t<AdjList>> facial_cycle = 
	{g.edge(0,1).value(),
	g.edge(1,2).value(),
	g.edge(2,3).value(),
	g.edge(3,4).value(),
	g.edge(4,5).value(),
	g.edge(5,0).value()
	};

	std::vector<bool> b1_attachments = {false,true,false,false,true,false};
	std::vector<bool> b2_attachments = {false,false,true,false,false,true};
	

	auto overlap = bridgeOverlap(g,b1_attachments,b2_attachments,facial_cycle);

	//for(auto&& v : overlap.value())
	//	std::cout << v << ' ';
	//std::cout << std::endl;
	ASSERT(overlap.size()==4);

}

auto test_bridgeOverlap3(){
	auto g = IndexedGraph{genCycle<AdjList>(6)};

	std::vector<edge_t<AdjList>> facial_cycle = 
	{g.edge(0,1).value(),
	g.edge(1,2).value(),
	g.edge(2,3).value(),
	g.edge(3,4).value(),
	g.edge(4,5).value(),
	g.edge(5,0).value()
	};

	std::vector<bool> b1_attachments = {false,true,false,false,true,false};
	std::vector<bool> b2_attachments = {false,false,true,false,false,false};
	

	auto overlap = bridgeOverlap(g,b1_attachments,b2_attachments,facial_cycle);

	ASSERT(overlap.size()==0);

}


int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_bridgeOverlap();
	test_bridgeOverlap2();
	test_bridgeOverlap3();
}
