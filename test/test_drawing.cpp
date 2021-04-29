#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/is_straight_line_drawing.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/draw.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_cpdrawing(){

	auto g = GraphWrapper{genCycle<AdjList>(5)};

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));

	pg = makeMaximal(std::move(pg));

	auto dg = chrobakPayneDraw(pg);
	
	auto coordinates_pmap = make_iterator_property_map(dg.coordinates.begin(),get(boost::vertex_index,dg.getGraph()));

	ASSERT(is_straight_line_drawing(dg.getGraph(),coordinates_pmap));
}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_cpdrawing();

}
