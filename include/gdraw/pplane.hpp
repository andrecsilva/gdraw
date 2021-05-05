#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <limits>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include <gdraw/graph_types.hpp>
#include <gdraw/util.hpp>
#include <gdraw/planar_graphs.hpp>

namespace gdraw{

/*
 * Checks wheter the cycle is 1-sided (if the number of negative signal edges is odd)
 */
template <typename Graph,int Genus,EdgeRange<Graph> T>
auto is1Sided(const NonOrientableEmbeddedGraph<Graph,Genus>& g, const T& cycle) -> bool{
	int signal=1;

	for(auto&& e : cycle){
		//std::cout << e <<std::endl;
		signal *= g.signal(e);
	}

	return signal == -1? true: false;
}


template <typename Graph, typename Range>
requires EdgeRange<Range,Graph>
auto doubleCover(IndexedGraph<Graph>&& g,Range&& xedges) -> IndexedGraph<Graph>{

	//printGraph(g);
	auto n = num_vertices(g.getGraph());
	auto ecount = num_edges(g.getGraph());
	auto edgei_map = get( boost::edge_index, g.getGraph());

	//TODO ugly solution... would like to devise something a bit better using the indexes
	//Adding edges invalidate iterators, we need a cache
	std::vector<edge_t<Graph>> edges_cache;
	for(auto&& e : range(edges(g.getGraph()))){
		edges_cache.push_back(e);
	}

	for([[maybe_unused]]auto&& _ : std::views::iota((size_t)0,n)){
		add_vertex(g.getGraph());
	}

	auto endpoints = [&g](auto e){
		return std::make_tuple(source(e,g.getGraph()),target(e,g.getGraph()));
	};

	//Makes a copy of the original g on the vertices with index n,2n-1
	for(auto&& [s,t] : edges_cache | std::views::transform(endpoints)){
		auto f = add_edge(s+n,t+n,g.getGraph()).first;
		boost::put(edgei_map,f,ecount++);
	}

	for(auto&& e : xedges){
		auto [s,t] = endpoints(e);

		auto f = edge(s+n,t+n,g.getGraph()).first;

		remove_edge(e,g.getGraph());
		remove_edge(s+n,t+n,g.getGraph());

		auto e_index = boost::get(edgei_map,e);
		auto f_index = boost::get(edgei_map,f);

		auto f1 = add_edge(s,vertex(t+n,g.getGraph()),g.getGraph()).first;
		auto f2 = add_edge(vertex(s+n,g.getGraph()),t,g.getGraph()).first;
		boost::put(edgei_map,f1,e_index);
		boost::put(edgei_map,f2,f_index);
	}

	return g;
}


template <typename Graph>
auto findDoublePlanarCover(IndexedGraph<Graph> g) -> std::optional<PlanarGraph<Graph>>{

	auto tree_edges = randomSpanningTree(g);

	auto cotree_edges = coSubgraphEdges(g,tree_edges);

	std::optional<PlanarGraph<Graph>> maybe_planar;

	auto has_dpc = [&maybe_planar,&g](auto&& edges){
		auto [h,h_edges] = graph_copy(g,edges);
		auto dc = doubleCover(std::move(h),std::move(h_edges));
		auto v = planeEmbedding(std::move(dc));
		if(std::holds_alternative<PlanarGraph<Graph>>(v)){
			maybe_planar = std::move(std::get<0>(v));
			return true;
		}
		return false;
		
	};

	size_t max_size = cotree_edges.size();

	size_t min_size = std::min(cotree_edges.size() - 2*num_vertices(g.getGraph()) + 5,(size_t)0);

	gdraw::enumerate(min_size,max_size,cotree_edges,has_dpc);

	return maybe_planar;
}


/*
 * parent - a parent vector representing a rooted tree.
 * root - root of the tree represented by parent.
 */
template <typename Graph>
auto doublePlanarCover(ProjectivePlanarGraph<Graph> g, std::vector<vertex_t<Graph>> parent, vertex_t<Graph> root) -> PlanarGraph<Graph>{
	//We fix a spanning tree T and check for edges e such that the unique cycle in T+e is non-contractible
	//These edges are used to calculate the double cover 

	std::vector<int> signal_to_root(num_vertices(g.getGraph()),0);

	signal_to_root[root]=1;

	std::vector<vertex_t<Graph>> stack;
	
	auto edgei_map = get( boost::edge_index, g.getGraph());

	//Here we calculate the signal of the path from the vertex to the root
	for(auto&& v : range(vertices(g.getGraph()))){
		auto w = v;
		while(signal_to_root[w] == 0 or w != root){
			stack.push_back(w);
			w = parent[w];
		}
		while(!stack.empty()){
			auto u = stack.back();
			stack.pop_back();
			auto e = edge(u,w,g.getGraph()).first;
			signal_to_root[u] = signal_to_root[w] * g.edge_signals[get(edgei_map,e)];
			w = u;
		}
	}

	std::vector<edge_t<Graph>> tree_edges;

	for(auto v : range(vertices(g.getGraph())))
		if(parent[v] != boost::graph_traits<Graph>::null_vertex())
			tree_edges.push_back(edge(v,parent[v],g.getGraph()).first);
	
	auto cotree_edges = coSubgraphEdges(g,tree_edges);

	auto endpoints = [&g](auto&& e){
		return std::make_tuple(source(e,g.getGraph()),target(e,g.getGraph()));
	};

	std::vector<edge_t<Graph>> xedges;

	//The paths may have a a common subpath to root, but since it it counted twice it doesn't matter (Z/2Z)
	for(auto&& e : cotree_edges){
		auto [u,v] = endpoints(e);
		auto e_signal = g.edge_signals[get(edgei_map,e)];
		if(signal_to_root[u] * signal_to_root[v] * e_signal == -1)
			xedges.push_back(e);
	}

	//TODO need to make an embedding from the xedges...
	auto dc = doubleCover(std::move(g),xedges);
	auto maybe_planar_emb = planeEmbedding(std::move(dc));

	return std::get<PlanarGraph<Graph>>(maybe_planar_emb);

}

template <typename Graph>
auto doublePlanarCover(ProjectivePlanarGraph<Graph> g) -> PlanarGraph<Graph>{

	std::vector<vertex_t<Graph>> parent(num_vertices(g.getGraph()),boost::graph_traits<Graph>::null_vertex());

	boost::mt19937 gen(time(0));
	boost::random_spanning_tree(g.getGraph(),gen,boost::predecessor_map(&parent[0]));

	auto root = *(vertices(g.getGraph()).first);

	return doublePlanarCover(std::move(g),parent,root);

}

template <typename Graph>
auto embeddingFromDPC(PlanarGraph<Graph> g) -> ProjectivePlanarGraph<Graph>{
	
	//Due to the complexity of the operations clearVertex and remove_vertex, creating a new graph is simply faster
	auto n = num_vertices(g.getGraph());
	Graph h = Graph(n/2);

	auto endpoints = [&g](auto&& e){
		return std::make_tuple(source(e,g.getGraph()),target(e,g.getGraph()));
	};

	auto is_x_edge = [&n,&endpoints](auto&& e){
		auto [u,v] = endpoints(e);
		if((u < n/2 && v>=n/2) ||
			(u >=n/2 && v < n/2))
			return true;
		return false;
	};

	auto project = [&n](auto&& v){
		return v % (n/2);
	};


	//We need to maintain the multiplicity of edges
	//We first count the number of edges that project to to a single edge
	//Each edge will have twice their multiplicity as projections
	std::map<std::tuple<vertex_t<Graph>,vertex_t<Graph>>,size_t> count_projections;
	for(auto&& e : range(edges(g.getGraph()))){
		auto [u,v] = endpoints(e);
		u = project(u);
		v = project(v);
		++count_projections[std::make_tuple(u,v)];
	}

	auto edgei_map = boost::get(boost::edge_index,h);
	size_t h_ecount =0;

	//We now add exactly half the multiplicity of the projected edges
	for(auto&& [p,e_count] : count_projections){
		//std::cout << e << ' ' << e_count << std::endl;
		//auto [u,v] = endpoints(e);
		auto [u,v] = p;
		//std::cout << u << ' ' << v << ' ' << e_count << std::endl;
		for([[maybe_unused]]auto&& _ : std::views::iota((size_t)0,e_count/2)){
			auto f = add_edge(u,v,h).first;
			boost::put(edgei_map,f,h_ecount++);
		}
	}

	rotations_t<Graph> h_rotations(n/2);
	std::vector<int> edge_signals(num_edges(h),1);

	for(size_t u =0;u<n/2;u++)
		for(auto&& e : g.rotations[u]){
			auto [v,w] = endpoints(e);
			edge_t<Graph> f;
			if(is_x_edge(e)){
				v = project(v);
				w = project(w);
				f = edge(v,w,h).first;
				auto f_index = boost::get(edgei_map,f);
				h_rotations[u].push_back(f);
				edge_signals[f_index] = -1;
			}else{
				auto f = edge(v,w,h).first;
				auto f_index = boost::get(edgei_map,f);
				h_rotations[u].push_back(f);
				edge_signals[f_index] = 1;
			}
		}

	return ProjectivePlanarGraph<Graph>{std::move(h),std::move(h_rotations),std::move(edge_signals)};
}


}//namespace gdraw
