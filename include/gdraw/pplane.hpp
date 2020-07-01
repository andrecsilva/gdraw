#ifndef PPLANE_HPP
#define PPLANE_HPP

#include <iostream>
#include <vector>
#include <map>
#include <limits>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "graph_types.hpp"
#include "util.hpp"

namespace gdraw{

//Type for embedding scheme
//Maybe edge_t instead of std::pair?
//template <typename Graph>
using embedding_t = typename std::vector<std::vector<std::pair<size_t,int>>>;

//Reads embeddings from Myrvold and Campbell's program
embedding_t readEmbedding(){
	size_t ngraph = 0;
	size_t nvertices = 0;

	std::cin >> ngraph;
	std::cin >> nvertices;

	embedding_t embedding {nvertices};

	for(size_t i=0; i< nvertices; i++){
		size_t degree=0;
		std::cin >> degree;	

		embedding.at(i) = std::vector<std::pair<size_t,int>>{degree};
		for (size_t j =0; j<degree; j++){
			size_t vertex=0;
			int signal=0;
			std::cin >> vertex >> signal;
			embedding.at(i).at(j) = std::make_pair(vertex,signal);
		}

	}
	return embedding;
}

//template <typename Graph, template<typename> typename EdgeContainer,typename Edge>
//Graph doubleCover(const Graph& g, const EdgeContainer<Edge>& xedges){
//	std::vector<bool> mask = std::vector<bool>(xedges.size(),true);
//	doubleCover(g,xedges,mask);
//}
/*
 * Returns a double cover of g from a collection of edges.
 * The parameter mask specifies 
 * TODO return the projection?
 * TODO mask -> ranges/views in C++20
 */
//template <typename Graph, template<typename> typename EdgeContainer,typename Edge>
template <typename Graph>
Graph doubleCover(const Graph& g, const std::vector<edge_t<Graph>>& xedges,std::vector<bool> mask){
	//copy graph with edges;
	Graph h(g);

	auto n = num_vertices(g);
	//projections:
	//auto proj_v = [&n,&g](vertex_t<Graph> v){return vertex(v % n,g); };
	//auto proj_e = [&n,&g](edge_t<Graph> e){return edge(source(e,g) % n,target(e,g) % n).first; };


	for(size_t i=0;i<n;i++)
		add_vertex(h);

	auto h_ecount = num_edges(h);
	auto edgei_map = boost::get(boost::edge_index,h);

	//Another copy of g, but on the newer vertices
	for(auto [ei,ei_end] = edges(g); ei!=ei_end;ei++){
		auto u = target(*ei,g);
		auto v = source(*ei,g);
		auto f = add_edge(vertex(u+n,h),vertex(v+n,h),h).first;
		boost::put(edgei_map,f,h_ecount++);
	}

	//Removes both copies of e in xedges from h and adds two edges
	//between both copies
	
	for(size_t i=0; i<xedges.size(); i++){
		auto e = xedges[i];
		if(mask[i]){
			auto u = target(e,g);
			auto v = source(e,g);

			auto f = edge(u+n,v+n,h).first;

			auto e_index = boost::get(edgei_map,e);
			auto f_index = boost::get(edgei_map,f);

			remove_edge(u,v,h);
			remove_edge(vertex(u+n,h),vertex(v+n,h),h);

			auto f1 = add_edge(u,vertex(v+n,h),h).first;
			auto f2 = add_edge(vertex(u+n,h),v,h).first;

			boost::put(edgei_map,f1,e_index);
			boost::put(edgei_map,f2,f_index);
		}
	}

	return h;
}

template <typename Graph>
std::vector<edge_t<Graph>> getCrossEdges(const Graph& dpc,const size_t inv){
	std::vector<edge_t<Graph>> xedges;

	for(auto [ei,ei_end]= edges(dpc); ei!=ei_end;ei++){
		auto u = source(*ei,dpc);
		auto v = target(*ei,dpc);
		if((u < inv && v >=inv) ||
				(v < inv && u >=inv  ))
			xedges.push_back(*ei);
	}

	return xedges;
}

/*
 * Returns an embedding of G in the Projective Plane from a
 * double planar cover.
 */
template <typename Graph>
std::tuple<rotations_t<Graph>,std::vector<int>>
embeddingFromDPC(const Graph& g, const Graph& dpc){
	rotations_t<Graph> embedding(num_vertices(g));

	rotations_t<Graph> embedding_dpc(num_vertices(dpc));

	boyer_myrvold_planarity_test(
			boost::boyer_myrvold_params::graph = dpc
			,boost::boyer_myrvold_params::embedding = &embedding_dpc[0]
			);
	auto n = num_vertices(g);

	std::vector<edge_t<Graph>> xedges = getCrossEdges(dpc,n);

	auto edgei_map = boost::get(boost::edge_index,g);
	
	std::vector<int> edge_signals(num_edges(g),1);
	auto edge_signals_map =  make_iterator_property_map(edge_signals.begin(),edgei_map);

	for(size_t i =0; i<n; i++){
		for(auto& e : embedding_dpc[i]){
			auto u = target(e,dpc);
			auto v = source(e,dpc);
			int signal = 1;
			if(u < n && v >=n){
				v-=n;
				signal = -1;
			}
			if(v < n && u >=n){
				u-=n;
				signal = -1;
			}
			auto f = edge(u,v,g).first;
			//std::cout << "[" << boost::get(edgei_map,f) << "]" << f << " " << signal << std::endl;
			embedding[i].push_back(f);
			boost::put(edge_signals_map,f,signal);
		}
	}

	return {embedding,edge_signals};
}

/*
 * Finds a double planar cover, if one exists.
 * Exponential on the number of edges of g, but should be quite fast.
 */

template <typename Graph>
std::tuple<bool,Graph> findDoublePlanarCover(const Graph& g){
	Graph dpc;
	auto tree_edges = randomSpanningTree(g);

	auto cotree_edges = coSubgraphEdges(g,tree_edges);

	size_t max_size = cotree_edges.size();
	//For the double cover to be planar
	size_t min_size = std::min(cotree_edges.size() - 2*num_vertices(g) + 5,(size_t)0);
	//std::cout << min_size << ' ' << max_size << ' ' << cotree_edges.size() << std::endl;

	auto isDPC = [](Graph& g,Graph& h, auto& xedges,std::vector<bool>& mask){
		h = doubleCover(g,xedges,mask);
		bool isPlanar =
			boyer_myrvold_planarity_test(
					boost::boyer_myrvold_params::graph = h
					);

		//if(isPlanar)
		//	printGraph(h);

		return isPlanar;

	};

	auto execute = std::bind(isDPC,g,std::ref(dpc),std::placeholders::_1,std::placeholders::_2);

	bool found = boundedSubsetsExecute<std::vector<edge_t<Graph>>>(cotree_edges,execute,min_size,max_size);

	//for(auto e : tree_edges)
	//	std::cout << e << ' ';
	//std::cout << " Spannning tree" << std::endl;

	return {found,dpc};
}

/*
 * Finds an embedding of g in the projective plane, if it exists.
 * The embedding will be stored on out_rotations.
 * Exponential on the number of edges of g, but should be quite fast.
 */
template <typename Graph>
bool findProjectiveEmbedding(const Graph& g, std::tuple<rotations_t<Graph>,std::vector<int>> out_embedding){

	auto [found,dpc] = findDoublePlanarCover(g);

	if(found)
		out_embedding = embeddingFromDPC(g,dpc);

	return found;
}

//TODO function to find minimal double cover -> min cross edges
template <typename Graph>
bool findMinDoubleCover(const Graph& g);

}

#endif //PPLANE_HPP

