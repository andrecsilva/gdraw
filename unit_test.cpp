#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <boost/graph/adjacency_list.hpp>

#include "include/gdraw/util.hpp"
#include "include/gdraw/xnumber.hpp"
#include "include/gdraw/pplane.hpp"

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

BOOST_AUTO_TEST_SUITE (isPlanar_test)

BOOST_AUTO_TEST_CASE(isPlanar_test)
{
	AdjList K5 = gdraw::getKn<AdjList>(5);

	rotations_t<AdjList> rotations(num_vertices(K5));
	std::vector< edge_t<AdjList> > kuratowski_edges;

	BOOST_CHECK(!gdraw::isPlanar(K5,kuratowski_edges,rotations));
	
	std::cout << "K5 is not planar - edges of the Kuratowski subgraph: " << std::endl;
	for(auto ei = kuratowski_edges.begin(); ei!= kuratowski_edges.end(); ++ei)
		std::cout << *ei << " ";
	std::cout << std::endl << std::endl;

	boost::remove_edge(0,1,K5);

	BOOST_CHECK(gdraw::isPlanar(K5,kuratowski_edges,rotations));

	std::cout << "K5-e is planar, embedding:" << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}
}

BOOST_AUTO_TEST_SUITE_END ()

BOOST_AUTO_TEST_SUITE (XNumber1)

BOOST_AUTO_TEST_CASE(XNumber1_test)
{
	AdjList g = gdraw::getKpq<AdjList>(3,4);

	rotations_t<AdjList> rotations;
	std::vector<edge_t<AdjList>> kuratowski_edges;

	BOOST_CHECK(gdraw::leqXnumberk(g,rotations,1)==false);
	
	std::cout << "cr(K3,4) > 1"  << std::endl;

	g = gdraw::getKpq<AdjList>(3,3);

	BOOST_CHECK(gdraw::leqXnumberk(g,rotations,1)==true);

	std::cout << "cr(K3,3) = 1, embedding: " << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

	g = gdraw::getKn<AdjList>(4);

	BOOST_CHECK(gdraw::leqXnumberk(g,rotations,1)==true);

	std::cout << "cr(K4) = 0, embedding: " << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}
}

BOOST_AUTO_TEST_SUITE_END ()

BOOST_AUTO_TEST_SUITE (XNumberGeneral)

BOOST_AUTO_TEST_CASE(XNumberGeneral_test)
{
	AdjList g = gdraw::getKpq<AdjList>(3,4);

	rotations_t<AdjList> rotations;
	std::vector<edge_t<AdjList>> kuratowski_edges;

	BOOST_CHECK(gdraw::leqXnumberk(g,rotations,2)==true);
	
	std::cout << "cr(K3,4) <= 2"  << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

	g = gdraw::getKn<AdjList>(6);

	BOOST_CHECK(gdraw::leqXnumberk(g,rotations,2)==false);

	std::cout << "cr(K6) > 2" << std::endl;

	g = gdraw::getKn<AdjList>(6);

	BOOST_CHECK(gdraw::leqXnumberk(g,rotations,3)==true);

	std::cout << "cr(K6) <= 3, embedding: " << std::endl;

	gdraw::removeIsolatedVertices(g);

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

}

BOOST_AUTO_TEST_SUITE_END ()

BOOST_AUTO_TEST_SUITE (DoublePlanarCover)

BOOST_AUTO_TEST_CASE(DoublePlanarCover_test){

	AdjList g = gdraw::getKpq<AdjList>(3,3);

	auto [found,h] = gdraw::findDoublePlanarCover(g);

	BOOST_CHECK(found==true);

	auto h_projection =  [&g](edge_t<AdjList> e){
		auto v = source(e,g);
		auto w = target(e,g);
		auto n = num_vertices(g);

		return edge(v % n ,w % n,g);

	};

	BOOST_CHECK(2*num_vertices(g) == num_vertices(h));

	for(auto [ei,ei_end] = edges(h); ei!=ei_end; ei++)
		BOOST_CHECK(h_projection(*ei).second==true);

	g = gdraw::getKn<AdjList>(7);

	std::tie(found,h) = gdraw::findDoublePlanarCover(g);

	BOOST_CHECK(found==false);

}
BOOST_AUTO_TEST_SUITE_END ()


//BOOST_AUTO_TEST_SUITE (DoublePlanarCover)
//
//BOOST_AUTO_TEST_CASE(DoublePlanarCover_test){
//
//	AdjList h = planarDoubleCover(g,esignals);
//
//}
//BOOST_AUTO_TEST_SUITE_END ()

