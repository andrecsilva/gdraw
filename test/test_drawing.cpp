#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/is_straight_line_drawing.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/draw.hpp>

#include <gdraw/io.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_laplacian(){
	auto g = IndexedGraph<AdjList>{getKpq<AdjList>(4,4)};
	auto L = laplacian(g);

	for(auto&&e : g.edges()){
		auto [u,v] = g.endpoints(e);
		auto [i,j] = std::make_tuple(g.index(u),g.index(v));
		ASSERT(L(i,j) == L(j,i));
		ASSERT(L(i,j) == -1);
	}

	for(auto&& v : g.vertices()){
		auto i = g.index(v);
		ASSERT(L(i,i) == g.degree(v));
	}
}

auto test_tuttedrawing1(){
	auto g = IndexedGraph<AdjList>{getKpq<AdjList>(2,3)};
	auto L = laplacian(g);
	
	std::vector<vertex_t<AdjList>> cycle {0,2,1,3};
	std::vector<coord_t> cycle_coordinates = {{0,-2},{2,0},{0,2},{-2,0}};

	auto coordinates = tutteDrawImpl(g,cycle,cycle_coordinates);

	ASSERT(coordinates[4].x ==0 && coordinates[4].y ==0);
}

auto test_intersect(){

	coord_t a = {1,1};
	coord_t b = {-1,-1};
	coord_t c = {1,-1};
	coord_t d = {-1,1};

	ASSERT(intersect(a,b,c,d));
}

auto test_sldrawing(){

	auto g = IndexedGraph{genCycle<AdjList>(5)};

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));

	pg = makeMaximal(std::move(pg));

	auto dg = chrobakPayneDraw(pg);
	
	ASSERT(isStraightLineDrawing(dg));
}

auto test_cpdrawing(){

	auto g = IndexedGraph{genCycle<AdjList>(5)};

	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));

	pg = makeMaximal(std::move(pg));

	auto dg = chrobakPayneDraw(pg);
	
	auto coordinates_pmap = make_iterator_property_map(dg.coordinates.begin(),get(boost::vertex_index,dg.getGraph()));

	ASSERT(is_straight_line_drawing(dg.getGraph(),coordinates_pmap));
}

auto test_tuttedrawing2(){
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

	//std::vector<vertex_t<AdjList>> cycle {0,5,6};
	//std::vector<coord_t> cycle_coordinates = {{-3,-2},{3,-2},{0,3}};

	//auto coordinates = tutteDrawImpl(g,cycle,cycle_coordinates);

	//auto dg = DrawnGraph<AdjList>(std::move(g),std::move(coordinates));
	auto pg = std::get<PlanarGraph<AdjList>>(planeEmbedding(g));
	auto dg = tuttePlanarDraw(std::move(pg));
	
	//auto coordinates_pmap = make_iterator_property_map(coordinates.begin(),get(boost::vertex_index,g.getGraph()));
	//writeDOT(dg);
	//ASSERT(isStraightLineDrawing(dg));
	ASSERT(isStraightLineDrawing(dg));
	//ASSERT(is_straight_line_drawing(g.getGraph(),coordinates_pmap));

}

int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_cpdrawing();
	test_laplacian();
	test_sldrawing();
	test_intersect();
	test_tuttedrawing1();
	test_tuttedrawing2();
}
