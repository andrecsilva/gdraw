#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include "util.hpp"

BOOST_AUTO_TEST_SUITE (isPlanar_test)

BOOST_AUTO_TEST_CASE(isPlanar_test)
{
	Graph K5 = getKn(5);

	rotations_t<Graph> rotations(num_vertices(K5));
	std::vector< edge_t<Graph> > kuratowski_edges;

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
	Graph g = getKpq(3,4);

	rotations_t<Graph> rotations;
	std::vector<edge_t<Graph>> kuratowski_edges;

	BOOST_CHECK(leqXnumber1(g,rotations)==false);
	
	std::cout << "cr(K3,4) > 1"  << std::endl;

	g = getKpq(3,3);

	BOOST_CHECK(leqXnumber1(g,rotations)==true);

	std::cout << "cr(K3,3) = 1, embedding: " << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

	g = getKn(4);

	BOOST_CHECK(leqXnumber1(g,rotations)==true);

	std::cout << "cr(K4) = 0, embedding: " << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}
}

BOOST_AUTO_TEST_SUITE_END ()

//BOOST_AUTO_TEST_SUITE (isPlanar_test)
//
//BOOST_AUTO_TEST_CASE(isPlanar_test)
//{
//
//}
