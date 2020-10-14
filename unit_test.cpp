#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Hello
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>
#include <gdraw/xnumber.hpp>
#include <gdraw/pplane.hpp>
#include <gdraw/planar_graphs.hpp>

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

BOOST_AUTO_TEST_SUITE (isPlanar_test)

BOOST_AUTO_TEST_CASE(isPlanar_test)
{
	auto k5 = GraphWrapper<AdjList>{getKn<AdjList>(5)};

	auto v = planeEmbedding(k5);

	BOOST_CHECK(std::holds_alternative<NonPlanarGraph<AdjList>>(v));

	//std::cout << "K5 is not planar - edges of the Kuratowski subgraph: " << std::endl;

	boost::remove_edge(0,1,k5.getGraph());

	v = planeEmbedding(std::move(k5));

	BOOST_CHECK(std::holds_alternative<PlanarGraph<AdjList>>(v));

	//printEmbedding(std::get<0>(v));

}

BOOST_AUTO_TEST_SUITE_END ()

BOOST_AUTO_TEST_SUITE (PlanarXNumber)

BOOST_AUTO_TEST_CASE(PlanarXNumber)
{
	auto g = GraphWrapper<AdjList>{gdraw::getKpq<AdjList>(3,4)};
	auto h = GraphWrapper<AdjList>{gdraw::getKpq<AdjList>(3,3)};
	auto k = GraphWrapper<AdjList>{gdraw::getKn<AdjList>(6)};
	auto l = GraphWrapper<AdjList>{gdraw::getKn<AdjList>(4)};

	BOOST_CHECK(!planarXNumber(std::move(g),1));
	BOOST_CHECK(planarXNumber(std::move(h),2));
	BOOST_CHECK(planarXNumber(std::move(k),3));
	BOOST_CHECK(planarXNumber(std::move(l),1));
}

BOOST_AUTO_TEST_SUITE_END ()

BOOST_AUTO_TEST_SUITE (DoubleCover)

BOOST_AUTO_TEST_CASE(DoubleCover_test){

	auto g = GraphWrapper<AdjList>{gdraw::getKpq<AdjList>(3,3)};
	add_edge(0,1,g.getGraph());

	std::vector<edge_t<AdjList>> xedges = {edge(0,3,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(2,5,g.getGraph()).first};

	auto n = num_vertices(g.getGraph());
	auto m = num_edges(g.getGraph());

	auto dc = doubleCover(std::move(g),std::move(xedges));

	auto nc = num_vertices(dc.getGraph());
	auto mc = num_edges(dc.getGraph());

	BOOST_CHECK(nc = 2*n);
	BOOST_CHECK(mc = 2*m);
}
BOOST_AUTO_TEST_SUITE_END ()

BOOST_AUTO_TEST_SUITE (DoublePlanarCover)

BOOST_AUTO_TEST_CASE(DoublePlanarCover_test){

	auto g = GraphWrapper<AdjList>{gdraw::getKn<AdjList>(6)};

	auto result = findDoublePlanarCover(g);

	BOOST_CHECK(result);
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

