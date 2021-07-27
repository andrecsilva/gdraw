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
 * Returns a view containing the vertices of a given cycle in g.
 */
template <typename T,typename Graph>
requires std::ranges::forward_range<T> && EdgeRange<T,Graph>
auto inline cycleVertices(T& cycle, const IndexedGraph<Graph>& g){
	auto find_common = [&g](auto e,auto f){
		auto [v,w] = g.endpoints(e);
		auto [x,y] = g.endpoints(f);
		if(v==x || v==y)
			return v;
		if(w==x || w==y)
			return w;
		return IndexedGraph<Graph>::nullVertex();
	};

	auto next_vertex = [&cycle,ei=cycle.begin(),&g,&find_common](__attribute__((unused)) auto _) mutable{
		auto next = ei+1==cycle.end()?cycle.begin():ei++;
		return find_common(*ei,*next);
	};

	return cycle | std::views::transform(next_vertex);
}

/**
 * Returns a view containing pairs (e,v) such that v is the vertex in common between e and the next edge in the ycle.
 */
template <typename T,typename Graph>
requires std::ranges::forward_range<T> && EdgeRange<T,Graph>
auto inline cycleEdgeVertex(const T& cycle, const IndexedGraph<Graph>& g){
	auto find_common = [&g](auto e,auto f){
		auto [v,w] = g.endpoints(e);
		auto [x,y] = g.endpoints(f);
		if(v==x || v==y)
			return v;
		if(w==x || w==y)
			return w;
		return IndexedGraph<Graph>::nullVertex();
	};

	auto next_vertex = [&cycle,ei=cycle.begin(),&g,&find_common](auto e) mutable{
		auto next = ei+1==cycle.end()?cycle.begin():ei++;
		return std::make_tuple(e,find_common(*ei,*next));
	};

	return cycle | std::views::transform(next_vertex);
}

/**
 * Returns a view of all the tree edges. 
 */
auto inline treeEdges(const auto& tree){
	return filterOptional(tree);
}

/**
 * Recursive implementation of depth-first search in a graph. 
 * Returns the edges of a dfs Tree rooted at root.
 */
template <typename Graph>
void dfsTreeVisit(const IndexedGraph<Graph>& g,
	       	const vertex_t<Graph>& u,
		const vertex_t<Graph>& root,
	       	std::vector<std::optional<edge_t<Graph>>>& dfs_edges
		){

	//std::cout << u << std::endl;
	for(auto&& e : range(g.incidentEdges(u))){
		auto [a,b] = g.endpoints(e);
		auto v = a!=u ? a : b;
		if(v!=root && !dfs_edges[g.index(v)]){
			dfs_edges[g.index(v)]=e;
			dfsTreeVisit(g,v,root,dfs_edges);
		}
	}
}

/**
 * Returns a depth-first search forest of a graph represented by a vector where each position i contains the DFSForest edge of vertex i incident with its parent.
 */
template <typename Graph>
auto dfsForest(const IndexedGraph<Graph>& g, const vertex_t<Graph>& root){

	std::vector<std::optional<edge_t<Graph>>> dfs_edges(g.numVertices());

	dfsTreeVisit(g,root,root,dfs_edges);

	for(auto&& u : g.vertices()){
		if(!dfs_edges[g.index(u)]){
			dfsTreeVisit(g,u,u,dfs_edges);
		}
	}

	//for(auto&& p : parent)
	//	std::cout << p << ' ';
	//std::cout << std::endl;

	return dfs_edges;
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
		const std::vector<std::optional<edge_t<Graph>>>& tree,
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


template <typename Graph>
auto dfsBridgesVisit(const IndexedGraph<Graph>& g,
	       	const std::vector<bool>& in_subgraph,
		std::vector<size_t>& dfs_number,
		size_t& dfs_count,
		std::vector<edge_t<Graph>>& bridge_edges,
	       	vertex_t<Graph> u) -> void{
	//std::cout << u << std::endl;

	for(auto&& e : g.incidentEdges(u)){
		//grab the other endpoint
		auto [a,b] = g.endpoints(e);
		auto v = a!=u ? a : b;

		if(dfs_number[g.index(u)] > dfs_number[g.index(v)])
			bridge_edges.push_back(e);

		if(dfs_number[g.index(v)]==std::numeric_limits<size_t>::max() &&
				!in_subgraph[g.index(v)]){
			dfs_number[g.index(v)] = dfs_count++;
			dfsBridgesVisit(g,in_subgraph,dfs_number,dfs_count,bridge_edges,v);
		}
	}
}

/**
 * Returns a list containing all the bridges of a g. The bridges are represented as a list of edges.
 */
template <typename Graph>
auto bridges(const IndexedGraph<Graph>& g, const std::vector<edge_t<Graph>>& subgraph){

	//returns a vertex list of all the bridges of subgraph
	std::vector<bool> vertices_in_subgraph(g.numVertices(),false);
	std::vector<bool> edges_in_subgraph(g.numEdges(),false);

	std::vector<size_t> dfs_number(g.numVertices(),std::numeric_limits<size_t>::max());
	size_t dfs_count = 1;

	//std::cout << std::boolalpha << (f==subgraph[0]) << std::endl;
	for(auto&& e : subgraph){
		auto [a,b] = g.endpoints(e);
		vertices_in_subgraph[g.index(a)]=true;
		vertices_in_subgraph[g.index(b)]=true;
		edges_in_subgraph[g.index(e)]=true;
		dfs_number[g.index(a)] = dfs_count++;
		dfs_number[g.index(b)] = dfs_count++;
	}

	std::vector<std::vector<edge_t<Graph>>> bridges;

	for(auto&& v : g.vertices()){
		if(dfs_number[v]==std::numeric_limits<size_t>::max()){
			std::vector<edge_t<Graph>> bridge_edges;
			dfs_number[v] = dfs_count++;
			dfsBridgesVisit(g,vertices_in_subgraph,dfs_number,dfs_count,bridge_edges,v);
			bridges.push_back(bridge_edges);
		}
	}

	//add trivial bridges
	for(auto&& e : g.edges()){
		if(!edges_in_subgraph[g.index(e)]){
			//grab the other endpoint
			auto [a,b] = g.endpoints(e);
			if(vertices_in_subgraph[g.index(a)] && vertices_in_subgraph[g.index(b)])
				bridges.push_back({e});
		}
	}
	
	return bridges;
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
template<typename Graph,typename T>
requires EdgeRange<T,Graph>
auto coSubgraphEdges(const GraphWrapper<Graph>& g, T&& subgraph_edges){
	std::vector<edge_t<Graph>> cs_edges;

	auto edgei_map = get(boost::edge_index,g.getGraph());

	std::vector<bool> in_subgraph (num_edges(g.getGraph()),false);
	auto in_subgraph_map = make_iterator_property_map(in_subgraph.begin(), edgei_map);

	for(auto&& e : subgraph_edges){
		put(in_subgraph_map,e,true);
	}

	auto not_in_subgraph = [&in_subgraph_map](auto e){
		return !get(in_subgraph_map,e);
	};

	for(auto&& e : range(edges(g.getGraph())) | std::views::filter(not_in_subgraph))
			cs_edges.push_back(e);

	return cs_edges;
}
/**
 * Creates a subgraph based on an edge list.
 * Returns a tuple containing the an IndexedGraph representing
 * the subgraph and a SubgraphMap containing the maps from its
 * vertices and edges to g.
 */
template <typename Graph,typename T>
requires EdgeRange<T,Graph>
auto createSubgraph(IndexedGraph<Graph>& g, T&& edge_list){

	std::vector<std::optional<vertex_t<Graph>>> vertex_map(g.numVertices());

	//An edge map from subg to g
	std::vector<edge_t<Graph>> iedge_map;

	IndexedGraph<Graph> subg{Graph()};

	for(auto&& e: edge_list){
		auto [a,b] = g.endpoints(e);

		if(!vertex_map[g.index(a)])
			vertex_map[g.index(a)] = subg.addVertex();
		if(!vertex_map[g.index(b)])
			vertex_map[g.index(b)] = subg.addVertex();

		auto x = vertex_map[g.index(a)].value();
		auto y = vertex_map[g.index(b)].value();

		vertex_map[g.index(a)] = x;
		vertex_map[g.index(b)] = y;
		subg.addEdge(x,y);
		iedge_map.push_back(e);
	}
	std::vector<vertex_t<Graph>> ivertex_map(subg.numVertices());

	for(size_t i=0; i< vertex_map.size(); i++){
		if(vertex_map[i]){
			auto v = vertex_map[i].value();
			ivertex_map[subg.index(v)] = g.vertex(i);
		}
	}

	return std::make_tuple(subg,
			SubgraphMap{g,std::move(ivertex_map),
			std::move(iedge_map)});
}

/**
 * Checks if two bridges overlap. The input parameters are characteristic vectors of their attachments. The result will be a std::vector of size 4 (skew overlap), 3 or 0 (no overlap) containing the overlapping attachments.
 */
template <typename T,typename Graph>
requires std::ranges::forward_range<T> && EdgeRange<T,Graph>
auto bridgeOverlap(const IndexedGraph<Graph> g,
		const std::vector<bool> b1_attachments,
		const std::vector<bool> b2_attachments,
		T&& cycle) -> std::vector<vertex_t<Graph>>{

	//overlap
	std::vector<vertex_t<Graph>> overlap;
	for(auto&& v : cycleVertices(cycle,g)){
		if(b1_attachments[g.index(v)] && b2_attachments[g.index(v)]){
			overlap.push_back(v);
			if(overlap.size()==3)
				return overlap;
		}
	}

	auto find_common = [&g](auto e,auto f){
		auto [v,w] = g.endpoints(e);
		auto [x,y] = g.endpoints(f);
		if(v==x || v==y)
			return v;
		if(w==x || w==y)
			return w;
		return IndexedGraph<Graph>::nullVertex();
	};

	auto next_vertex = [&cycle,&g,&find_common](auto ei){
		auto next = ei+1==cycle.end()?cycle.begin():ei++;
		return find_common(*ei,*next);
	};


	vertex_t<Graph> first;
	auto first_iterator = cycle.begin();
	for(;first_iterator!=cycle.end();first_iterator++){
		auto v = next_vertex(first_iterator);
		if(b1_attachments[v] || b2_attachments[v]){
			first = v;
			break;
		}
	}

	if(first_iterator==cycle.end())
		return {};

	bool inb1 = b1_attachments[first];
	overlap = {first};

	//std::cout << first << ' ' << *first_iterator << std::endl;
	auto ei = first_iterator+1==cycle.end()?cycle.begin():first_iterator+1;

	do{
		auto v = next_vertex(ei);
		//std::cout << v << ' ' << *ei << std::endl;
		if(inb1){
			if(b2_attachments[g.index(v)]){
				overlap.push_back(v);
				if(overlap.size()==4)
					return overlap;
				inb1=!inb1;
			}
		}else{
			if(b1_attachments[g.index(v)]){
				overlap.push_back(v);
				if(overlap.size()==4)
					return overlap;
				inb1=!inb1;
			}
		}
		ei = ei+1==cycle.end()? cycle.begin() : ei+1;
	}while(ei!=first_iterator);

	return {};
}

/**
 * Returns true if a is a subset of b.
 */
auto isSubset(const std::vector<bool>& a, const std::vector<bool>& b) -> bool{
	for(size_t i =0;i<a.size();i++){
		if(a[i] && !b[i])
			return false;
	}
	return true;
}

/**
 * Return a characteristic vector of all the attachments of bridge.
 * in_subg is the characteristic vector of the vertice set of the subgraph
 * that bridge is a bridge.
 */
template <typename Graph>
auto attachments(const std::vector<edge_t<Graph>>& bridge, const std::vector<bool>& in_subg, const IndexedGraph<Graph>& g){
	std::vector<bool> is_bridge_attachment(g.numVertices(),false);
	for(auto&& e : bridge){
		auto&& [a,b] = g.endpoints(e);
		is_bridge_attachment[g.index(a)] = in_subg[g.index(a)];
		is_bridge_attachment[g.index(b)] = in_subg[g.index(b)];
	}
	return is_bridge_attachment;
}

/**
 * Returns a characteristic vector indicating if a vertex is incident with
 * an edge of subg.
 **/
template <typename T,typename Graph>
requires std::ranges::forward_range<T> && EdgeRange<T,Graph>
auto vertices(T&& subg,const IndexedGraph<Graph>& g){
	std::vector<bool> in_subg(g.numVertices(),false);
	for(auto&& e : subg){
		auto&& [a,b] = g.endpoints(e);
		in_subg[g.index(a)]=true;
		in_subg[g.index(b)]=true;
	}
	return in_subg;
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


void dfsSCC1(const std::vector<std::vector<int>>& graph, int v,std::vector<int>& order,std::vector<bool>& visited){
	visited[v]=true;
	for(auto&& w : graph[v])
		if(!visited[w])
			dfsSCC1(graph,w,order,visited);	

	order.push_back(v);
}

void dfsSCC2(const std::vector<std::vector<int>>& graph, int v,int comp,std::vector<int>& component){
	component[v]=comp;
	for(auto&& w : graph[v])
		if(component[w]==-1)
			dfsSCC2(graph,w,comp,component);	
}

/**
 * An implementation of Kosaraju's algorithm for a graph represented by an adjacency matrix.
 */
auto stronglyConnectedComponents(const std::vector<std::vector<int>>& graph){

	std::vector<bool> visited (graph.size(),false);
	std::vector<int> order; 

	for(size_t i=0 ;i < graph.size(); i++){
		if(!visited[i]){
			dfsSCC1(graph,i,order,visited);
		}
	}

	std::vector<std::vector<int>> graph_t(graph.size());

	for(size_t u=0; u<graph.size();u++)
		for(auto&& v : graph[u])
			graph_t[v].push_back(u);

	size_t comp =0;
	std::vector<int> component (graph.size(),-1);

	for(size_t i=0 ;i < graph.size(); i++){
		int v = order[graph.size() - i - 1];
		if(component[v]==-1){
			dfsSCC2(graph_t,v,comp,component);
			comp++;
		}
	}

	return component;
}

} //namespace gdraw
