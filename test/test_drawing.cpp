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

auto test_build_system(){
	auto g = GraphWrapper{AdjList(5)};

	add_edge(0,1,g.getGraph());
	add_edge(0,2,g.getGraph());
	add_edge(0,3,g.getGraph());
	add_edge(1,2,g.getGraph());
	add_edge(1,3,g.getGraph());
	add_edge(1,4,g.getGraph());
	add_edge(2,3,g.getGraph());
	add_edge(2,4,g.getGraph());
	add_edge(3,4,g.getGraph());

	std::vector<vertex_t<AdjList>> facial_cycle = {0,1,2};

	std::vector<std::pair<vertex_t<AdjList>,coord_t>> drawn_cycle;
	drawn_cycle = {
		{0,{3,6}},
		{2,{4,1}},
		{1,{0,3}}
	};

	std::vector<bool> in_cycle(num_vertices(g.getGraph()),false);

	in_cycle[0] = true;
	in_cycle[1] = true;
	in_cycle[2] = true;

	auto [A,bx,by,vertex_to_row] = buildSystem(g.getGraph(),drawn_cycle);

	ASSERT(A(0,0) ==4);
	ASSERT(A(0,1) ==-1);
	ASSERT(A(1,0) ==-1);
	ASSERT(A(1,1) ==3);

	ASSERT(bx(0) = 7);
	ASSERT(bx(1) = 4);

	ASSERT(by(0) = 10);
	ASSERT(by(1) = 4);
}


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
	test_build_system();

}
