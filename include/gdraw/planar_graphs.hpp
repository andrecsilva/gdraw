/**
 * Functions for manipulating planar graphs.
 */
#pragma once

#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>

#include <gdraw/graph_types.hpp>

namespace gdraw{

/**
 * Add edges to make `g` maximal planar, that is, all faces are triangles.
 */	
template <typename Graph,int Genus>
auto makeMaximal(OrientableEmbeddedGraph<Graph,Genus> g) -> OrientableEmbeddedGraph<Graph,Genus>{
	auto edgei_map = get(boost::edge_index, g.getGraph());
	auto ecount = num_edges(g.getGraph());

	edge_t<Graph> original_0_edge;
	for(auto&& e : range(edges(g.getGraph())))
		if(get(edgei_map,e)==0)
			original_0_edge = e;

	auto rotations_pmap = make_iterator_property_map(g.rotations.begin(),get(boost::vertex_index,g.getGraph()));
	make_biconnected_planar(g.getGraph(),rotations_pmap);

	//Added edges have index 0, we need to fix those for the planarity algorithm
	auto has_index_0 = [&,original_0_edge,edgei_map](auto&& e){
		return (e != original_0_edge) && (get(edgei_map,e)==0);
	};

	for(auto&& e : range(edges(g.getGraph())) | std::views::filter(has_index_0))
		put(edgei_map,e,ecount++);

	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g.getGraph()
		,boost::boyer_myrvold_params::embedding = rotations_pmap
		);

	make_maximal_planar(g.getGraph(),rotations_pmap);

	for(auto&& e : range(edges(g.getGraph())) | std::views::filter(has_index_0))
		put(edgei_map,e,ecount++);

	//make sure the signals vector has the appropriate size and values
	g.edge_signals.resize(num_edges(g.getGraph()),1);

	//make sure the embedding is updated...
	
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g.getGraph()
		,boost::boyer_myrvold_params::embedding = rotations_pmap
		);
	
	return g;
}

template <typename Vertex>
struct FacialCyclesVisitor : public boost::planar_face_traversal_visitor{

	FacialCyclesVisitor(std::vector<std::vector<Vertex>>& facial_cycles) : facial_cycles(&facial_cycles) {};

	std::vector<std::vector<Vertex>>* facial_cycles;

	std::vector<Vertex> current_cycle;

	void begin_face(){
		current_cycle = {};	
		//std::cout << "New Face: " << std::endl;
	}

	void next_vertex(Vertex v){
		current_cycle.push_back(v);
		//std::cout << v << " ";
	}

	void end_face(){
		facial_cycles->push_back(std::move(current_cycle));
		//std::cout << std::endl;
	}

};


//TODO redo this without using boost and generalized to surfaces
/**
 * Finds a facial cycle in an embedded graph `g`.
 *
 * Just a wrap aroung Boost's `planar_face_traversal` function.
 */
template <typename Graph,int Genus>
auto findFacialCycle(const OrientableEmbeddedGraph<Graph,Genus>& g) -> std::vector<vertex_t<Graph>>{
	std::vector<std::vector<vertex_t<Graph>>> facial_cycles;
	FacialCyclesVisitor<vertex_t<Graph>> fcv {facial_cycles};
	auto rotations_pmap = make_iterator_property_map(g.rotations.begin(),get(boost::vertex_index,g.getGraph()));
	planar_face_traversal(g.getGraph(),rotations_pmap,fcv);
	return facial_cycles[0];
}

/**
 * Test wheter `g` is a planar graph.
 *
 * Finds either an embedding or Kuratowski subgraph as a side-effect.
 *
 * @return : A `std::variant` containing either the graph with an embedding (`PlanarGraph`) or with a list of edges that induces a Kuratowksi subgraph (`NonPlanarGraph`).
 */
template <typename Graph>
auto planeEmbedding(GraphWrapper<Graph> g) -> std::variant<PlanarGraph<Graph>,NonPlanarGraph<Graph>>{

	std::vector<edge_t<Graph>> kuratowski_edges;
	rotations_t<Graph> rotations(num_vertices(g.getGraph()));

	auto rotations_pmap = make_iterator_property_map(rotations.begin(),get(boost::vertex_index,g.getGraph()));

	bool embedded = boyer_myrvold_planarity_test(
			boost::boyer_myrvold_params::graph = g.getGraph()
			,boost::boyer_myrvold_params::embedding = rotations_pmap
			,boost::boyer_myrvold_params::kuratowski_subgraph = std::back_inserter(kuratowski_edges)
			);

	if(embedded){
		return OrientableEmbeddedGraph<Graph,0>(std::move(g),std::move(rotations),std::vector<int>(num_edges(g.getGraph()),1));
	}
	return NonEmbeddableGraph<Graph>(std::move(g),std::move(kuratowski_edges));
}


}//namespace

