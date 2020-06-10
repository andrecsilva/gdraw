#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <boost/graph/adjacency_list.hpp>

#include "util.hpp"
#include "xnumber.hpp"
#include "pplane.hpp"

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
	AdjList K5 = getKn<AdjList>(5);

	rotations_t<AdjList> rotations(num_vertices(K5));
	std::vector< edge_t<AdjList> > kuratowski_edges;

	BOOST_CHECK(!isPlanar(K5,kuratowski_edges,rotations));
	
	std::cout << "K5 is not planar - edges of the Kuratowski subgraph: " << std::endl;
	for(auto ei = kuratowski_edges.begin(); ei!= kuratowski_edges.end(); ++ei)
		std::cout << *ei << " ";
	std::cout << std::endl << std::endl;

	boost::remove_edge(0,1,K5);

	BOOST_CHECK(isPlanar(K5,kuratowski_edges,rotations));

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
	AdjList g = getKpq<AdjList>(3,4);

	rotations_t<AdjList> rotations;
	std::vector<edge_t<AdjList>> kuratowski_edges;

	BOOST_CHECK(leqXnumberk(g,rotations,1)==false);
	
	std::cout << "cr(K3,4) > 1"  << std::endl;

	g = getKpq<AdjList>(3,3);

	BOOST_CHECK(leqXnumberk(g,rotations,1)==true);

	std::cout << "cr(K3,3) = 1, embedding: " << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

	g = getKn<AdjList>(4);

	BOOST_CHECK(leqXnumberk(g,rotations,1)==true);

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
	AdjList g = getKpq<AdjList>(3,4);

	rotations_t<AdjList> rotations;
	std::vector<edge_t<AdjList>> kuratowski_edges;

	BOOST_CHECK(leqXnumberk(g,rotations,2)==true);
	
	std::cout << "cr(K3,4) <= 2"  << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

	g = getKn<AdjList>(6);

	BOOST_CHECK(leqXnumberk(g,rotations,2)==false);

	std::cout << "cr(K6) > 2" << std::endl;

	g = getKn<AdjList>(6);

	BOOST_CHECK(leqXnumberk(g,rotations,3)==true);

	std::cout << "cr(K6) <= 3, embedding: " << std::endl;

	removeIsolatedVertices(g);

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

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
