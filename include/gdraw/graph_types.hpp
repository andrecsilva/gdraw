#pragma once

#include <vector>
#include <map>
#include <ranges>
#include <memory>
#include <iostream>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

#include <gdraw/coordinates.hpp>

namespace gdraw{
	
template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

template <typename Graph>	
using rotations_t = typename std::vector< std::vector< edge_t<Graph> > >;

template <typename T,typename Graph>
concept EdgeRange = std::ranges::range<T> && std::is_same<std::remove_cvref_t<std::ranges::range_reference_t<T>>,edge_t<Graph>>::value;

template <typename T,typename Graph>
concept VertexRange = std::ranges::range<T> && std::is_same<std::remove_cvref_t<std::ranges::range_reference_t<T>>,vertex_t<Graph>>::value;

constexpr bool debug = 0;

/**
 * Auxiliary function that transforms the iterator pair from boost::edges into a proper viewable range.
 *
 * Mostly used internally.
 */
template <typename BeginIterator,typename EndIterator>
inline auto range(std::pair<BeginIterator,EndIterator>&& ipair){
	return std::ranges::subrange(ipair.first,ipair.second);
}

/**
 * Auxiliary function that transforms a range into a viewable range.
 *
 * Mostly used internally.
 */
template <typename Range>
inline auto range(Range&& r){
	return std::ranges::subrange(r.begin(),r.end());
}

/**
 * Returns the pair of endpoints of a Boost Graph.
 *
 * Mostly used internally.
 */
template <typename Graph>
inline auto endpoints(const Graph& g,const edge_t<Graph>& e){
	return std::make_tuple(source(e,g),target(e,g));
}

/**
 * This class wraps a boost graph class and implements move semantics using std::unique_ptr.
 *
 * The reason for the existence of this class is that Boost Graph objects do not play well with move semantics.
 *
 */
template <typename Graph>
class GraphWrapper{
	private:
		std::unique_ptr<Graph> base_graph;
	public:

		GraphWrapper(const Graph& base){
			base_graph = std::make_unique<Graph>(base);
		}

		GraphWrapper(const GraphWrapper& other){
			//base_graph = std::make_unique<Graph>(*other.base_graph);
			*this = other;
		}

		GraphWrapper(GraphWrapper&& other){
			if constexpr (debug)
			       	std::cout << "GraphWrapper move constructor" << std::endl;
		
			*this = std::move(other);
			//base_graph = std::move(other.base_graph);
			//other.base_graph = std::make_unique<Graph>();
		}

		GraphWrapper& operator=(GraphWrapper&& other){
			if constexpr (debug)
				std::cout << "GraphWrapper move assignment" << std::endl;
			base_graph = std::make_unique<Graph>();
			base_graph.swap(other.base_graph);
			return *this;
		}

		GraphWrapper& operator=(const GraphWrapper& other){
			if constexpr (debug)
				std::cout << "GraphWrapper copy assignment" << std::endl;
			base_graph = std::make_unique<Graph>(*other.base_graph);
			//base_graph = std::move(other.base_graph);
			return *this;
		}

		//TODO Ideally I'd like to make a implicity conversion
		//TODO Hard to make it work with boost
		//operator Graph&(){
		//	return *base_graph;
		//}

		inline Graph& getGraph(){
			return *base_graph;
		}

		inline Graph& getGraph() const{
			return *base_graph;
		}

};

template <template <typename> typename T,typename Graph>
concept AsGraphWrapper = std::convertible_to<T<Graph>,GraphWrapper<Graph>>;

//template<typename Graph>
//class HasGraphWrapper{
//
//	protected: 
//		HasGraphWrapper(GraphWrapper<Graph> g) : graph(std::move(g)){}
//		GraphWrapper<Graph> graph;
//	public:
//		Graph& getGraph() const{
//			return graph.getGraph();
//		}
//
//		Graph& getGraph(){
//			return graph.getGraph();
//		}
//		
//		//operator GraphWrapper<Graph>() const{
//		//	return graph;
//		//}
//		//
//		operator GraphWrapper<Graph>&(){
//			return graph;
//		}
//
//		operator GraphWrapper<Graph>&() const{
//			return graph;
//		}
//
//		//operator GraphWrapper<Graph>&&() const{
//		//	return std::move(graph);
//		//}
//};



/** An abstract class used for the implementation of `OrientableEmbeddedGraph` and `NonOrientableEmbeddedGraph`
 *
 * You probaly do not want to use this.
 */
template <typename Graph>
class EmbeddedGraph : public GraphWrapper<Graph>{
	public:
		rotations_t<Graph> rotations;

		EmbeddedGraph(GraphWrapper<Graph> g, rotations_t<Graph> rotations)
		: GraphWrapper<Graph>(std::move(g)),
		rotations(std::move(rotations)){}

		EmbeddedGraph(EmbeddedGraph&& other) : GraphWrapper<Graph>(std::move(other)){
			if constexpr(debug)
				std::cout << "EmbeddedGraph Move build" << std::endl;
			(*this).rotations = std::move(other.rotations);
		}

		EmbeddedGraph(const EmbeddedGraph& other) : GraphWrapper<Graph>(other){
			if constexpr(debug)
				std::cout << "EmbeddedGraph Copy build" << std::endl;
			(*this).rotations = rotations_t<Graph>(num_vertices(other.getGraph()));
					
			auto this_edgei_map = get(boost::edge_index, (*this).getGraph());
			auto other_edgei_map = get(boost::edge_index, other.getGraph());

			std::vector<edge_t<Graph>> edges_by_index(num_edges((*this).getGraph()));

			//TODO where is edges defined?
			for(auto e : range(edges((*this).getGraph()))){
				auto ei = boost::get(this_edgei_map,e);
				edges_by_index[ei] = e;
			}
			
			//auto endpoints = [&other](auto e){
			//	return std::make_tuple(source(e,other.getGraph()),target(e,other.getGraph()));
			//};

			//for(auto&& pi_u : other.rotations){
			//	for(auto&& e : pi_u){
			//		auto [u,v] = endpoints(e);
			//		auto f = edge(u,v,other.getGraph()).first;
			//		std::cout << e << '[' << get(other_edgei_map,e)  << ',' << get(other_edgei_map,f) <<  ']' << ' ' ;
			//	}
			//	std::cout << std::endl;
			//}

			for(auto v =0;auto&& pi_v : other.rotations){
				for(auto&& e : pi_v){
					auto ei = boost::get(other_edgei_map,e);
					auto f = edges_by_index[ei];
					rotations[v].push_back(f);
				}
				v++;
			}
		}
		
		EmbeddedGraph& operator=(EmbeddedGraph&& other){
			if constexpr(debug)
				std::cout << "EmbeddedGraph Move =" << std::endl;
			GraphWrapper<Graph>::operator=(std::move(other));
			(*this).rotations = std::move(other.rotations);
			return *this;
		}

		EmbeddedGraph& operator=(const EmbeddedGraph& other){ 
			if constexpr(debug)
				std::cout << "Copy =" << std::endl;
			(*this) = EmbeddedGraph(other);
			return *this;
		}
};


/**
 * A Graph that is embedded in a orientable surface of genus `Genus`.
 */
template <typename Graph,int Genus>
class OrientableEmbeddedGraph : public EmbeddedGraph<Graph>{

	public:
		//rotations_t<Graph> rotations;
		//int genus = Genus;
		
		//OrientableEmbeddedGraph<Graph,Genus>(const OrientableEmbeddedGraph<Graph,Genus>& other) = default;

		//OrientableEmbeddedGraph<Graph,Genus>(OrientableEmbeddedGraph<Graph,Genus>&& other) = default;

		OrientableEmbeddedGraph(GraphWrapper<Graph> g, rotations_t<Graph> rotations):
			EmbeddedGraph<Graph>{std::move(g),std::move(rotations)}{}
			//,euler_characteristic(std::move(euler_characteristic)) {}

		inline auto signal(edge_t<Graph>) const -> int{
			return 1;
		}

		inline auto genus() -> int{
			return Genus;
		}
};

template <typename Graph,int Genus>
class NonOrientableEmbeddedGraph : public EmbeddedGraph<Graph>{

	public:
		std::vector<int> edge_signals;
		
		//TODO int range instead of vector?
		NonOrientableEmbeddedGraph(GraphWrapper<Graph> g, rotations_t<Graph> rotations, std::vector<int> edge_signals):
			EmbeddedGraph<Graph>{std::move(g),std::move(rotations)},
			edge_signals(std::move(edge_signals)){}

		//OrientableEmbeddedGraph(OrientableEmbeddedGraph& other) = default;
		//OrientableEmbeddedGraph(OrientableEmbeddedGraph&& other) = default;

		inline auto signal(edge_t<Graph> e) const -> int{
			auto edgei_map = get( boost::edge_index, this->getGraph());
			return edge_signals[get(edgei_map,e)];
		}

		inline auto genus() -> int{
			return Genus;
		}

};

//TODO include the genus template parameter on this class
/** 
 * A graph togheter with a list of edges that induces a forbidden subgraph.
 */
template <typename Graph>
class NonEmbeddableGraph : public GraphWrapper<Graph>{

	public:
		std::vector<edge_t<Graph>> forbidden_subgraph;

		NonEmbeddableGraph(GraphWrapper<Graph> g, std::vector<edge_t<Graph>> forbidden_subgraph):
			GraphWrapper<Graph>(std::move(g)),
			forbidden_subgraph(std::move(forbidden_subgraph)) {}

		NonEmbeddableGraph(NonEmbeddableGraph&& other) : GraphWrapper<Graph>(std::move(other)), forbidden_subgraph(std::move(other.forbidden_subgraph)){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Move Constructor" << std::endl;
		}

		NonEmbeddableGraph(const NonEmbeddableGraph& other) : GraphWrapper<Graph>(other){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Copy Constructor" << std::endl;
			auto this_edgei_map = get(boost::edge_index, (*this).getGraph());
			auto other_edgei_map = get(boost::edge_index, other.getGraph());

			std::vector<edge_t<Graph>> edges_by_index(num_edges((*this).getGraph()));

			//TODO where is edges defined?
			for(auto e : range(edges((*this).getGraph()))){
				auto ei = boost::get(this_edgei_map,e);
				edges_by_index[ei] = e;
			}

			for(auto&& e : other.forbidden_subgraph){
				auto ei = boost::get(other_edgei_map,e);
				(*this).forbidden_subgraph.push_back(edges_by_index[ei]);
			}

		}


		NonEmbeddableGraph& operator=(NonEmbeddableGraph&& other){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Move =" << std::endl;
			GraphWrapper<Graph>::operator=(std::move(other));
			(*this).forbidden_subgraph = std::move(other.forbidden_subgraph);
			return *this;
		}

		NonEmbeddableGraph& operator=(const NonEmbeddableGraph& other){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Copy =" << std::endl;
			*this = NonEmbeddableGraph(other);
			return *this;
		}
};

/**
 * A graph drawn on the plane.
 *
 * Edges are drawn as cubic splines and are stored on `edge_coordinates`. Edges whose coordinates are not include there should be interpreted as a line segment between its endpoints.
 */
template <typename Graph>
class DrawnGraph : public GraphWrapper<Graph>{

	public:
		std::vector<coord_t> coordinates;
		std::vector<std::string> vertex_colors;
		std::map<edge_t<Graph>,cubicSpline> edge_coordinates;
		std::map<edge_t<Graph>,std::string> edge_colors;

		DrawnGraph(GraphWrapper<Graph> g, std::vector<coord_t> coordinates):
			GraphWrapper<Graph>(std::move(g)),
			coordinates(std::move(coordinates)),
       			vertex_colors(num_vertices((*this).getGraph()),""){
				//std::cout << "Constructor DrawnGraph" << std::endl;
				//std::cout << "Size: " << vertex_colors.size() << std::endl;
				//std::cout << "Num_vertices: " << num_vertices(this->getGraph()) << std::endl;
			}

		DrawnGraph(GraphWrapper<Graph> g, std::vector<coord_t> coordinates,std::map<edge_t<Graph>,cubicSpline> edge_coordinates):
			GraphWrapper<Graph>(std::move(g)),
			coordinates(std::move(coordinates)),
       			vertex_colors(num_vertices((*this).getGraph()),""),
			edge_coordinates(std::move(edge_coordinates)){}

		DrawnGraph(DrawnGraph&& other):
		       	GraphWrapper<Graph>(std::move(other)),
       			vertex_colors(std::move(other.vertex_colors)),
			edge_coordinates(std::move(other.edge_coordinates)),
			edge_colors(std::move(other.edge_colors)){
				if constexpr(debug)
					std::cout << "DrawnGraph Move constructor" << std::endl;
			}

		DrawnGraph(const DrawnGraph& other) : GraphWrapper<Graph>(other){
			if constexpr(debug)
				std::cout << "DrawnGraph Copy constructor" << std::endl;
			coordinates = other.coordinates;
			vertex_colors = other.vertex_colors;

			auto this_edgei_map = get(boost::edge_index, (*this).getGraph());
			auto other_edgei_map = get(boost::edge_index, other.getGraph());

			std::vector<edge_t<Graph>> edges_by_index(num_edges((*this).getGraph()));

			//TODO where is edges defined?
			for(auto e : range(edges((*this).getGraph()))){
				auto ei = boost::get(this_edgei_map,e);
				edges_by_index[ei] = e;
			}

			for(auto&& [e,s] : other.edge_coordinates){
				auto ei = boost::get(other_edgei_map,e);
				auto f = edges_by_index[ei];
				std::cout << '[' << ei << ']' << e << ' ' << f << ' ' << s << std::endl;
				(*this).edge_coordinates[f] = s;
			}

			for(auto&& [e,c] : other.edge_colors){
				auto ei = boost::get(other_edgei_map,e);
				auto f = edges_by_index[ei];
				(*this).edge_colors[f] = c;
			}
		}

		DrawnGraph& operator=(DrawnGraph&& other){
			if constexpr(debug)
				std::cout << "DrawnGraph Move =" << std::endl;
			GraphWrapper<Graph>::operator=(std::move(other));
			(*this).edge_colors = std::move(other.edge_colors);
			(*this).edge_coordinates = std::move(other.edge_coordinates);
			(*this).vertex_colors = std::move(other.vertex_colors);
			(*this).coordinates = std::move(other.coordinates);
			return *this;
		}

		DrawnGraph& operator=(const DrawnGraph& other){
			if constexpr(debug)
				std::cout << "DrawnGraph Copy =" << std::endl;
			*this = DrawnGraph(other);
			return *this;
		}

		auto colorEdge(const edge_t<Graph>& e,std::string color){
			edge_colors[e] = std::move(color);

		}

		auto colorVertex(const vertex_t<Graph>& v, std::string color){
			//TODO maybe change this to explicitly use vertex_index map
			vertex_colors[v] = std::move(color);
		}
};

template <template <typename,int> typename T,typename Graph,int Genus>
concept Embeddable = std::is_same<T<Graph,Genus>,OrientableEmbeddedGraph<Graph,Genus>>::value ||
std::is_same<T<Graph,Genus>,NonOrientableEmbeddedGraph<Graph,Genus>>::value;

template <typename Graph>
using PlanarGraph = OrientableEmbeddedGraph<Graph,0>;

template <typename Graph>
using NonPlanarGraph = NonEmbeddableGraph<Graph>;

template <typename Graph>
using ProjectivePlanarGraph = NonOrientableEmbeddedGraph<Graph,1>;

/**
 * Returns a copy of g and g_edges such that the copy of g_edges are valid descriptors in the copied graph.
 *
 * Mostly used for debugging purposes or internally.
 * @return: a tuple containing a `GraphWrapper` and a `EdgeRange`
 */
auto graph_copy(const auto& g, const auto& g_edges){
	//decltype(g) h = g;
	auto h = g;

	using edge_t = typename std::remove_cvref<decltype(*(edges(h.getGraph()).first))>::type;
	//std::cout << typeid(edge_t).name() << std::endl;
	std::vector<edge_t> h_edges{};

	auto g_edgei_map = get(boost::edge_index, g.getGraph());
	auto h_edgei_map = get(boost::edge_index, h.getGraph());

	std::vector<edge_t> edges_by_index(num_edges(h.getGraph()));

	for(auto&& e : range(edges(h.getGraph()))){
		auto ei = boost::get(h_edgei_map,e);
		edges_by_index[ei] = e;
	}

	for(auto&& e : g_edges){
		auto ei = boost::get(g_edgei_map,e);
		h_edges.push_back(edges_by_index[ei]);
	}

	return std::make_tuple(std::move(h),std::move(h_edges));
}

/** Lists all the vertices and edges of a graph with its indexes.
 */
template <typename Graph>
void printGraph(const GraphWrapper<Graph>& g){
	std::cout << "Vertices: " << std::endl;
	for(auto&& v : range(vertices(g.getGraph())))
		std::cout << v << ' ';
	std::cout << std::endl;

	auto edgei_map = get( boost::edge_index, g.getGraph());

	std::cout << "Edges: " << std::endl;
	for(auto&& e : range(edges(g.getGraph())))
		std::cout << "[" << boost::get(edgei_map,e) << "]" << e << " "; 

	std::cout << std::endl;
}

/** Prints the embedding scheme of an embedded graph.
 */
template<template<typename,int> typename T,typename Graph, int Genus>
requires Embeddable<T,Graph,Genus>
auto printEmbedding(const T<Graph,Genus>& g){
	for(auto&& pi_u : g.rotations){
		for(auto&& e : pi_u){
			std::cout << (g.signal(e)==1?'+':'-') << e << ' ';
		}
		std::cout << std::endl;
	}
}

}//namespace