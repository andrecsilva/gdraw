#pragma once

#include <iostream>
#include <functional>
#include <utility>
#include <algorithm>
#include <ranges>

#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <cppitertools/combinations.hpp>
#include <gdraw/graph_types.hpp>

namespace gdraw{


//TODO maybe use parallel for each? : std::for_each(std::execution::par_unseq,...
/*
 * Executes `execute` for each subset of collection with size between `min_size` and `max_size` until it returns `true`.
 */
template <typename IterableCollection,typename BoolFunctionOverIterable>
auto enumerate(size_t min_size, size_t max_size, IterableCollection collection,BoolFunctionOverIterable execute) -> bool{

	//auto bound_range = std::ranges::reverse_view(std::ranges::iota_view{min_size,max_size});
	
	auto bound_range = std::ranges::iota_view{min_size,max_size+1};

	if(min_size==0){
		using value_t = std::remove_cvref<decltype(*(std::declval<IterableCollection>().begin()))>::type;
		std::ranges::empty_view<value_t> j;
		if(execute(j))
			return true;
	}

	for (auto&& i : bound_range){
		for (auto&& j : iter::combinations(collection,i)) {
			if(execute(j))
				return true;
		}
	}
	return false;
}

/**
 * Recursive implementation of depth-first search in a graph.
 */
template <typename Graph>
void dfsTreeVisit(const Graph& g, const vertex_t<Graph>& u, std::vector<vertex_t<Graph>>& parent){

	//std::cout << u << std::endl;
	for(auto&& v : range(adjacent_vertices(u,g))){
		if(parent[v] == boost::graph_traits<Graph>::null_vertex()){
			parent[v]=u;
			dfsTreeVisit(g,v,parent);
		}
	}
}

/**
 * Returns a depth-first search forest of a graph represented by a vector containing
 * the parent for each vertex.
 */
template <typename Graph>
auto dfsForest(const GraphWrapper<Graph>& g, vertex_t<Graph>& root){

	std::vector<vertex_t<Graph>> parent(num_vertices(g.getGraph()),boost::graph_traits<Graph>::null_vertex());
	parent[root] = root;

	dfsTreeVisit(g.getGraph(),root,parent);

	for(auto&& u : range(vertices(g.getGraph()))){
		if(parent[u] == boost::graph_traits<Graph>::null_vertex()){
			parent[u] = u;
			dfsTreeVisit(g.getGraph(),u,parent);
			parent[u] = boost::graph_traits<Graph>::null_vertex();
		}
	}

	parent[root] = boost::graph_traits<Graph>::null_vertex();

	//for(auto&& p : parent)
	//	std::cout << p << ' ';
	//std::cout << std::endl;

	return parent;
}

/*
 * Returns a random spanning tree of g.
 *
 * It is just a wrapper around Boost's function.
 */
template<template<typename> typename Wrapper,typename Graph>
requires AsGraphWrapper<Wrapper,Graph>
std::vector<edge_t<Graph>> randomSpanningTree(const Wrapper<Graph>& g){	

	std::vector<vertex_t<Graph>> parent(num_vertices(g.getGraph()),boost::graph_traits<Graph>::null_vertex());
	std::vector<edge_t<Graph>> tree_edges;

	boost::mt19937 gen(time(0));
	boost::random_spanning_tree(g.getGraph(),gen,boost::predecessor_map(&parent[0]));

	//auto root = *(vertices(g.getGraph())).first;

	for(auto v : range(vertices(g.getGraph())))
		if(parent[v] != boost::graph_traits<Graph>::null_vertex())
			tree_edges.push_back(edge(v,parent[v],g.getGraph()).first);

	return tree_edges;
}

/*
 * Take a set of edges of `g` and returns the edges not in the set.
 * 
 * @param g: A GraphWrapper
 */
template<template<typename> typename Wrapper,typename Graph,typename Range>
requires AsGraphWrapper<Wrapper,Graph> && EdgeRange<Range,Graph>
std::vector<edge_t<Graph>> coSubgraphEdges(const Wrapper<Graph>& g, const Range& subgraph_edges){
	std::vector<edge_t<Graph>> cs_edges;

	auto edgei_map = get(boost::edge_index,g.getGraph());

	std::vector<bool> in_subgraph (num_edges(g.getGraph()),false);
	auto in_subgraph_map = make_iterator_property_map(in_subgraph.begin(), edgei_map);

	for(auto& e : subgraph_edges){
		put(in_subgraph_map,e,true);
	}

	auto not_in_subgraph = [&in_subgraph_map](auto e){
		return !get(in_subgraph_map,e);
	};

	for(auto&& e : range(edges(g.getGraph())) | std::views::filter(not_in_subgraph))
			cs_edges.push_back(e);

	return cs_edges;
}

/*
 * Removes vertices of `g` with no edges.
 */
template <typename Graph>
inline auto removeIsolatedVertices(Graph& g){
	//Reverse order, as the indexes are rearranged in the graph to make it contiguous
	for(int i = num_vertices(g)-1; i>0 ;i--){
		auto vi = vertex(i,g);
		if(out_degree(vi,g)==0)
			remove_vertex(vi,g);
	}
}

} //namespace gdraw
