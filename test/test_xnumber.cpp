#include <iostream>
#include <cassert>


#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>
#include <gdraw/xnumber.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_planarXNumber()
{
	auto g = IndexedGraph<AdjList>{gdraw::getKpq<AdjList>(3,4)};

	ASSERT(!planarXNumber(std::move(g),1));
}

auto test_planarXNumber2()
{
	auto h = IndexedGraph<AdjList>{gdraw::getKpq<AdjList>(3,3)};

	ASSERT(planarXNumber(std::move(h),2));
}

auto test_planarXNumber3()
{
	auto k = IndexedGraph<AdjList>{gdraw::getKn<AdjList>(6)};

	ASSERT(planarXNumber(std::move(k),3));
}

auto test_planarXNumber4()
{
	auto l = IndexedGraph<AdjList>{gdraw::getKn<AdjList>(4)};

	ASSERT(planarXNumber(std::move(l),1));
}



int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_planarXNumber();
	test_planarXNumber2();
	test_planarXNumber3();
	test_planarXNumber4();
}
