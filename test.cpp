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
// cut along operation for 1-sided cycles
// cut along for 2-sided cycles
// facial drawing for pplanar graphs
// test for separable cycle
// test for contractible cycle
// find the smallest non-contractible cycle
// 2-connected to 3-connected planar graph -> add a vertex connected to each vertex of the facial cycle (should get a bit better result with Tutte's Embedding)
// function to load an embedding from a file
// generator: graph and vertex_pair <- takes care of indexes for you
// Documentation
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
// Unit Tests
// Readme in github
// test for 1-sided cycle

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

/*
 * Cuts along a cycle in an embedded graph. The result is a new graph
 * where for each vertex v of the cycle is split into two vertices.
 * Their neighbors are determined by which "side" of the cycle they are.
 */
template <typename Graph,EdgeRange<Graph> T>
auto cutAlongCycle(EmbeddedGraph<Graph> g, const T& two_sided_cycle){
	
	std::vector<edge_t<Graph>> to_remove;
	auto find_common = [&g](auto e,auto f){
		auto [v,w] = gdraw::endpoints(g.getGraph(),e);
		auto [x,y] = gdraw::endpoints(g.getGraph(),f);
		if(v==x || v==y)
			return v;
		if(w==x || w==y)
			return w;
		return boost::graph_traits<Graph>::null_vertex();
	};
	auto get_next_edge = [&two_sided_cycle](auto ei){
		if(ei+1 == two_sided_cycle.end())
			return *(two_sided_cycle.begin());
		return *(ei+1);
	};
	auto is_left = [](auto i,auto ei,auto fi){
		if(ei< fi){
			return i<ei || i>fi;
		}
			return fi<i && i<ei;
	};
	
	auto edgei_map = get(boost::edge_index,g.getGraph());
	auto ecount = num_edges(g.getGraph());

	//TODO make new rotations
	for(auto&& ei = two_sided_cycle.begin();ei!=two_sided_cycle.end();ei++){
		auto e = *ei;
		auto f = get_next_edge(ei);
		auto u = find_common(e,f);
		
		std::cout << e << ' ' << f << ' ' << u << std::endl;
		size_t e_index =0;
		size_t f_index =0;
		for(size_t i =0;i<g.rotations[u].size();++i){
			auto h = g.rotations[u][i];
			if(h==e)
				e_index=i;
			if(h==f)
				f_index=i;
		}

		std::cout << e_index << ' ' << f_index << ' ' << std::endl;
		auto ul = add_vertex(g.getGraph());

		for(size_t i =0;i<g.rotations[u].size();++i){
			auto h = g.rotations[u][i];
			auto [v,w] = endpoints(g.getGraph(),h);
			auto not_u = u!=v? v : w;

			std::cout << h << ' ' << not_u << ' ' << std::endl;

			if(is_left(i,e_index,f_index)){
				auto k =  add_edge(ul,not_u,g.getGraph()).first;
				boost::put(edgei_map,k,boost::get(edgei_map,h));
				to_remove.push_back(h);
			}
			if(h==e || h==f){
				auto k =  add_edge(ul,not_u,g.getGraph()).first;
				boost::put(edgei_map,k,ecount++);
			}
		}
	}

	printGraph(g);

	for(auto&& e : to_remove)
		remove_edge(e,g.getGraph());

	printGraph(g);
}

int main(){
	GraphWrapper<AdjList> g {gdraw::getKpq<AdjList>(3,3)};
	GraphWrapper<AdjList> h {gdraw::getKn<AdjList>(6)};
	
	rotations_t<AdjList> rotations = {
		{edge(0,3,g.getGraph()).first,edge(0,4,g.getGraph()).first,edge(0,5,g.getGraph()).first},
		{edge(1,5,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(1,3,g.getGraph()).first},
		{edge(2,3,g.getGraph()).first,edge(2,5,g.getGraph()).first,edge(2,4,g.getGraph()).first},
		{edge(3,1,g.getGraph()).first,edge(3,2,g.getGraph()).first,edge(3,0,g.getGraph()).first},
		{edge(4,0,g.getGraph()).first,edge(4,2,g.getGraph()).first,edge(4,1,g.getGraph()).first},
		{edge(5,0,g.getGraph()).first,edge(5,2,g.getGraph()).first,edge(5,1,g.getGraph()).first}
	};

	auto edgei_map = get(boost::edge_index, g.getGraph());
	std::vector<int> esignals(num_edges(g.getGraph()),1);

	esignals[boost::get(edgei_map,edge(1,4,g.getGraph()).first)] = -1;
	esignals[boost::get(edgei_map,edge(2,5,g.getGraph()).first)] = -1;

	auto pg = ProjectivePlanarGraph<AdjList>(std::move(g),std::move(rotations),std::move(esignals));
	printGraph(pg);
	gdraw::printEmbedding(pg);

	std::vector<edge_t<AdjList>> cycle1 = {edge(0,3,pg.getGraph()).first,edge(3,1,pg.getGraph()).first,edge(1,4,pg.getGraph()).first,edge(4,0,pg.getGraph()).first};
	std::vector<edge_t<AdjList>> cycle2 = {edge(0,3,pg.getGraph()).first,edge(3,2,pg.getGraph()).first,edge(2,4,pg.getGraph()).first,edge(4,0,pg.getGraph()).first};
	//std::cout << std::boolalpha << gdraw::is1Sided(pg,cycle1) << std::endl;
	//std::cout << std::boolalpha << gdraw::is1Sided(pg,cycle2) << std::endl;

	cutAlongCycle(std::move(pg),cycle1);

}
