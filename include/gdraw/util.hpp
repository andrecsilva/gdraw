#pragma once

#include <iostream>
#include <functional>
#include <utility>
#include <algorithm>
#include <ranges>
#include <optional>

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
auto dfsForest(const IndexedGraph<Graph>& g, const vertex_t<Graph>& root){

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

template <typename Graph>
auto dfsFindCycleVisit(const IndexedGraph<Graph>& g,
	       	const vertex_t<Graph>& u,
	       	std::vector<vertex_t<Graph>>& parent) -> std::optional<std::tuple<vertex_t<Graph>,edge_t<Graph>>>{ 

	//std::cout << u << std::endl;
	for(auto&& e : g.incidentEdges(u)){
		//std::cout << e << std::endl;
		auto [a,b] = g.endpoints(e);
		auto v = a!=u ? a : b;
		if(parent[v] != IndexedGraph<Graph>::nullVertex()){
		       if(parent[u] != v)
			       return {{u,e}};
	       	}else{
			parent[v]=u;
			auto me = dfsFindCycleVisit(g,v,parent);
			if(me)
				return me.value();
		}
	}
	return {};
}

/**
 * Returns a bfs tree.
 */
template <typename Graph>
auto bfsTree(const IndexedGraph<Graph>& g, vertex_t<Graph> root){
	//finds the other endpoint of e that is not u
	auto other_endpoint = [&g](auto e,auto u){
		auto [a,b] = g.endpoints(e);
		return  a!=u? a : b;
	};
	std::list<vertex_t<Graph>> queue;
	queue.push_back(root);

	//tree edges incident with the vertices
	std::vector<std::optional<edge_t<Graph>>> bfs_edges(g.numVertices());
	bfs_edges[g.index(root)] = {};

	std::vector<edge_t<Graph>> fundamental_edges;

	while(!queue.empty()){
		auto u = queue.front();
		//std::cout << u << std::endl;
		queue.pop_front();

		for(auto&& e : g.incidentEdges(u)){
			auto v = other_endpoint(e,u);
			if(v!=root){
				auto f = bfs_edges[g.index(v)];
				if(!f){
					queue.push_back(v);
					bfs_edges[g.index(v)] = e;
				}
			}
		}
	}

	
	return bfs_edges;
}

/**
 * Returns the fundamental cycle obtained from a rooted tree and an edge.
 */
template <typename Graph>
auto fundamentalCycle(const IndexedGraph<Graph>& g,
		const std::vector<std::optional<edge_t<Graph>>> tree,
		const edge_t<Graph> e){

	//assumes v is an ancestor of u
	auto path_to_vertex = [&g,&tree](auto u,auto v){
		std::vector<edge_t<Graph>> path;
		auto parent = u;
		while(parent!=v){
			auto tree_edge = tree[parent].value();
			path.push_back(tree_edge);
			auto [a,b] = g.endpoints(tree_edge);
			parent = a!=parent? a : b;
		}
		return path;
	};

	auto [u,v] = g.endpoints(e);

	std::vector<bool> u_ancestor(g.numVertices(),false);
	u_ancestor[g.index(u)] = true;

	std::optional<edge_t<Graph>> tree_edge = tree[u];
	auto parent = u;

	while(tree_edge){
		//std::cout << tree_edge.value() << std::endl;
		//get parent
		auto [a,b] = g.endpoints(tree_edge.value());
		parent = a!=parent? a : b;

		u_ancestor[g.index(parent)] = true;

		tree_edge = tree[parent];
	}

	tree_edge = tree[v];
	parent = v;

	while(!u_ancestor[g.index(parent)]){
		//get parent
		//std::cout << parent << std::endl;
		auto [a,b] = g.endpoints(tree_edge.value());
		parent = a!=parent? a : b;
		tree_edge = tree[parent];
	}

	auto& common_ancestor = parent;

	auto u_path = path_to_vertex(u,common_ancestor);
	auto v_path = path_to_vertex(v,common_ancestor);

	std::reverse(v_path.begin(),v_path.end());
	
	std::move(v_path.begin(),v_path.end(),std::back_inserter(u_path));
	u_path.push_back(e);
	return u_path;
}


/**
 * Using a tree in parent format, traces a cycle from a back edge
 */
template <typename Graph>
auto buildCycle(const IndexedGraph<Graph>& g, const vertex_t<Graph> u, const edge_t<Graph>& back_edge, const std::vector<vertex_t<Graph>>& parent){
	//returns a path from u to v, assuming v is an ancestor of u, or to root otherwise
	auto path_to_vertex = [&g,&parent](auto u,auto v){
		std::vector<vertex_t<Graph>> path;
		vertex_t<Graph> p = parent[u];
		path.push_back(u);
		while(p != v){
			path.push_back(p);
			p = parent[p];
		}
		path.push_back(p);
		return path;
	};

	auto [a,b] = g.endpoints(back_edge);
	auto v = a!=u ? a : b;
	
	auto u_path = path_to_vertex(u,v);

	//std::cout << "u_path" << std::endl;
	//for(auto&& v : u_path)
	//	std::cout << v << ' ';
	//std::cout << std::endl;

	return u_path;
}

/**
 * Finds a cycle in a graph.
 */
template <typename Graph>
auto findCycle(const IndexedGraph<Graph>& g) -> std::optional<std::vector<vertex_t<Graph>>>{

	auto root = *(std::ranges::begin(g.vertices()));
	std::vector<vertex_t<Graph>> parent(g.numVertices(),IndexedGraph<Graph>::nullVertex());

	parent[root]=root;
	auto me = dfsFindCycleVisit(g,root,parent);
	parent[root]=IndexedGraph<Graph>::nullVertex();

	if(me){
		auto [u,e] = me.value();
		return buildCycle(g,u,e,parent);
	}

	parent[root]=root;
	for(auto&& u : g.vertices()){
		if(parent[u] == IndexedGraph<Graph>::nullVertex()){
			parent[u] = u;
			auto me = dfsFindCycleVisit(g,u,parent);
			parent[u] = IndexedGraph<Graph>::nullVertex();
			if(me){
				auto [v,e] = me.value();
				return buildCycle(g,v,e,parent);
			}
		}
	}
	parent[root]=IndexedGraph<Graph>::nullVertex();

	return {};
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
