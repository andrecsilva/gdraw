#include <iostream>
#include <functional>
#include <utility>
#include <vector>
#include <ranges>
#include <algorithm>
#include <span>
#include <variant>
#include <type_traits>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/taus88.hpp>

#include <cppitertools/combinations.hpp>
#include <cppitertools/enumerate.hpp>

//#include "include/gdraw/iterator_pair_range.hpp"
#include <gdraw/graph_types.hpp>
#include <gdraw/util.hpp>
#include <gdraw/draw.hpp>
#include <gdraw/io.hpp>
#include <gdraw/xnumber.hpp>
#include <gdraw/generators.hpp>
#include <gdraw/pplane.hpp>


using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 

// TODO:
// generator: graph and vertex_pair <- takes care of indexes for you
// Tests
// Documentation
// Readme in github
// Planarity test without moves
// Refactor planarxnumber and xnumber to be more modular (lots of shared code between them)
// genus: rotations_t<Graph>,VertexSignals -> int
// See about the removal of AsGraphWrapper (problems with template deduction?)
// DONE:
// xNumber: Graph,k,f -> variant<EmbeddableGraph<Graph,Genus>, NonEmbeddableGraph<Graph,Genus>a>
// planarizeXNumber: EmbeddedGraph,VertexRange -> DrawnGraph
// findDoublePlanarCover: Graph -> Maybe(Graph)
// doubleCover: const Graph -> Graph
// doublePlanarCover: Graph -> Maybe(Graph)
// findFacialCycle: const EmbeddedGraph -> Edge_Range
// makeMaximal: EmbeddedGraph -> EmbeddedGraph
// embed: Graph -> variant(NonEmbeddableGraph,EmbeddedGraph)
// draw: EmbeddedGraph -> DrawnGraph
// writeDOT: variant<Graph,DrawnGraph> -> String
// PlanarXNumber algorithm
// Move/Copy operations on DrawnGraph and NonEmbeddableGraph
// WriteDOT -> commas after pos in edges and vertices
// Debug and fix findDoublePlanarCover -> problem: we permanently modify the graph when we call doubleCover
// Check if remove_edge in the code removes a single edges only...
// Individual programs (xnumber,finddpc,findpembedding)...
// Tidy up makefile

using namespace gdraw;


struct output_visitor : public boost::planar_face_traversal_visitor
{
	void begin_face() { std::cout << "New face: "; }
	void end_face() { std::cout << std::endl; }
};

struct vertex_output_visitor : public output_visitor
{
	template < typename Vertex > void next_vertex(Vertex v)
	{
		std::cout << v << " ";
	}
};
//auto graph_copy(const auto& g, const auto& g_edges){
//	//decltype(g) h = g;
//	auto h = g;
//
//	using edge_t = typename std::remove_cvref<decltype(*(edges(h.getGraph()).first))>::type;
//	std::cout << typeid(edge_t).name() << std::endl;
//	typename std::remove_cvref<decltype(g_edges)>::type h_edges{};
//
//	auto g_edgei_map = get(boost::edge_index, g.getGraph());
//	auto h_edgei_map = get(boost::edge_index, h.getGraph());
//
//	std::vector<edge_t> edges_by_index(num_edges(h.getGraph()));
//
//	for(auto&& e : range(edges(h.getGraph()))){
//		auto ei = boost::get(h_edgei_map,e);
//		edges_by_index[ei] = e;
//	}
//
//	for(auto&& e : g_edges){
//		auto ei = boost::get(g_edgei_map,e);
//		h_edges.push_back(edges_by_index[ei]);
//	}
//
//	std::cout << "Point" << std::endl;
//	return std::make_tuple(std::move(h),std::move(h_edges));
//}

int main(){
	GraphWrapper<AdjList> g {gdraw::getKpq<AdjList>(3,3)};
	GraphWrapper<AdjList> h {gdraw::getKn<AdjList>(6)};

	//std::vector<edge_t<AdjList>> g_edges = {edge(0,3,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(2,5,g.getGraph()).first};

	auto result = findDoublePlanarCover(h);

	if (result){
		printGraph(result.value());
		printEmbedding(result.value());
	}

	//How to deal with functions that accept an edgeranges?
	//e.g. doubleCover(g,range);
	//Copying g WILL INVALIDATE the descriptors of the range...
	//solution 1:
	//copy(const T g&, const Edgerange& range){
	//}
	//solution 2: make g a forward reference (i.e. disallow copy)


	//TODO Implicit conversion of GraphWrapper does not seems to work here...
	//add_edge(0,1,graph2);
	//gdraw::printGraph(graph);
	//for (auto e : range(edges(graph.getGraph())))
	//	std::cout << e << std::endl;
	
	//std::vector<edge_t<AdjList>> tree_edges = {edge(0,1,graph.getGraph()).first,edge(1,2,graph.getGraph()).first,edge(2,3,graph.getGraph()).first,edge(3,4,graph.getGraph()).first,edge(4,5,graph.getGraph()).first};
	//
	//std::vector<edge_t<AdjList>> xedges = {edge(0,3,graph.getGraph()).first,edge(1,4,graph.getGraph()).first,edge(2,5,graph.getGraph()).first};

	//std::cout << "Original Graph:" << std::endl;
	//printGraph(graph);

	//auto dpc = std::get<PlanarGraph<AdjList>>(planeEmbedding(doubleCover(std::move(graph),xedges)));

	//std::cout << "Double Cover:" << std::endl;
	//printGraph(dpc);

	//std::cout << "Projective planar graph:" << std::endl;
	//auto pg = embeddingFromDPC(std::move(dpc));
	//printGraph(pg);
	//printEmbedding(pg);

	//std::cout << "Double Cover:" << std::endl;
	//auto dpc_id = doublePlanarCover(std::move(pg));
	//printGraph(dpc_id);
	//printEmbedding(dpc_id);


	//auto op_planar = [](auto&& g){
	//	auto v = gdraw::planeEmbedding(std::move(g));
	//	std::optional<PlanarGraph<AdjList>> pg;
	//	if(std::holds_alternative<PlanarGraph<AdjList>>(v)){
	//		pg = std::move(std::get<0>(v));
	//	}else
	//		g = std::move(std::get<1>(v));
	//	return pg;
	//};

	//size_t k = 3;

	////auto result = gdraw::xNumber(std::move(h),k,op_planar);
	//auto result = gdraw::planarXNumber(std::move(h),k);
	//
	//if(result)
	//	printGraph(result.value());
	//else
	//	std::cout << "Nooooope" << std::endl;


	//auto pg2 = (op_planar(GraphWrapper{gdraw::getKn<AdjList>(4)}).value());
	//
	//auto dg = gdraw::drawFlattenedGraph(result.value(),6);
	//dg.colorEdge(edge(0,1,dg.getGraph()).first,"red");
	//dg.colorEdge(edge(5,3,dg.getGraph()).first,"blue");
	//gdraw::writeDOT(dg);
	//auto dg2 = gdraw::tutteDraw(std::move(pg2));

	////dg2 = std::move(dg);
	//std::cout << "After this" << std::endl;
	//printGraph(dg);
	//dg2 = dg;
	//printGraph(dg2);
	//gdraw::writeDOT(dg2);

	//auto edgei_map = get(boost::edge_index, dg2.getGraph());
	//for(auto&& [e,c] : dg2.edge_coordinates){
	//	auto [u,v] = endpoints(dg2.getGraph(),e);
	//	auto f = edge(u,v,dg2.getGraph()).first;
	//	std::cout << e << '[' << get(edgei_map,e)  << ',' << get(edgei_map,f) <<  ']' << ' ' ;
	//}
	//std::cout << std::endl;

	//printGraph(pg);
	//printEmbedding(pg);
	

	//std::cout << std::boolalpha << std::ranges::range<decltype(facial_cycle)> << std::endl;
	//std::cout << std::boolalpha << (std::is_same<std::remove_cvref_t<std::ranges::range_reference_t<decltype(facial_cycle)>>,vertex_t<AdjList>>::value) << std::endl;


	//auto dg = drawFlattenedGraph(result.value(),6,gdraw::tutteDraw<AdjList>);
	
	//auto pg = std::move(result.value());

	//auto k4 = GraphWrapper{gdraw::getKn<AdjList>(4)};
	//auto kpg = op_planar(k4).value();

	//pg = kpg;

	//auto pg_edgei_map = get(boost::edge_index, pg.getGraph());

	//printGraph(pg);

	//printEmbedding(pg);
	//

	//for(auto&& pi_u : pg.rotations){
	//	for(auto&& e : pi_u){
	//		auto [u,v] = endpoints(pg.getGraph(),e);
	//		auto f = edge(u,v,pg.getGraph()).first;
	//		std::cout << e << '[' << get(pg_edgei_map,e)  << ',' << get(pg_edgei_map,f) <<  ']' << ' ' ;
	//	}
	//	std::cout << std::endl;
	//}


	//std::vector<std::map<vertex_t<AdjList>,edge_t<AdjList>>> next_edge_vector(num_edges(pg.getGraph()));

	//auto next_edge = make_iterator_property_map(next_edge_vector.begin(),get(boost::edge_index,g.getGraph()));

	//auto edgei_map = get(boost::edge_index, pg.getGraph());

	//for(auto&& v : range(vertices(pg.getGraph()))){
	//	std::cout << "Vertex: " << v << std::endl;
	//	auto pi_begin = pg.rotations[v].begin();
	//	auto pi_end = pg.rotations[v].end();
	//	for(auto pi = pi_begin; pi!=pi_end; ++pi){
	//		edge_t<AdjList> e(*pi);
	//		auto [u,v] = endpoints(pg.getGraph(),e);
	//		auto f = edge(u,v,pg.getGraph()).first;
	//		std::cout << e << std::endl;
	//		std::cout << f << std::endl;
	//		std::cout << boost::get(edgei_map,e) << std::endl;
	//		std::cout << boost::get(edgei_map,f) << std::endl;
	//		std::map<vertex_t<AdjList>,edge_t<AdjList>> m = boost::get(next_edge,e);
	//		m[v] = boost::next(pi) == pi_end ? *pi_begin : *boost::next(pi);
	//		boost::put(next_edge, e, m);
	//	}
	//}

	//TODO move and copy semantics for Embeddable graphs -> update rotations descriptors on the se operations
	//auto facial_cycle = gdraw::findFacialCycle(pg);
	//for(auto&& i : facial_cycle)
	//	std::cout << i << ' ';
	//std::cout << std::endl;

	//auto dg = gdraw::drawFlattenedGraph(std::move(pg),6,gdraw::tutteDraw<AdjList>);
	//gdraw::writeDOT(dg);
	//

	//auto ipair = edges(pg.getGraph());

	//auto range = std::ranges::subrange(ipair.first,ipair.second);
	
	//for(auto&& e : range(edges(pg.getGraph())) | std::views::filter(accept)){
	//	std::cout << e << std::endl;
	//}

	//if(result)
	//	printGraph(pg);
	//else
	//	std::cout << "cr(G) > k" << std::endl;

	//auto xg = xNumber(graph,1,op_planar2);
	//printGraph(result.value());

}
