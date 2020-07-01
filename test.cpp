#include <iostream>
#include <string>
#include <math.h>
#include <time.h>
#include <functional>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/taus88.hpp>


#include "include/gdraw/pplane.hpp"
#include "include/gdraw/io.hpp"
#include "include/gdraw/draw.hpp"

using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 


//template <typename Graph>
//std::vector<edge_t<Graph>> randomSpanningTree(const Graph& g){
//	std::vector<bool> visited (num_vertices(g),false);
//	std::vector<edge_t<Graph>> tree_edges;
//	vertex_t<Graph> v = rand() % num_vertices(g);
//	srand (time(NULL));
//	visited[v]=true;
//	randomDFS(v,g,visited,tree_edges);
//
//	return tree_edges;
//}

template <typename Graph>
void printEmbedding(Graph& g,std::tuple<rotations_t<Graph>,std::vector<int>> embedding){

	auto [rotations,edge_signals] = embedding;

	auto edgei_map = boost::get(boost::edge_index,g);
	auto edge_signals_map =  boost::make_iterator_property_map(edge_signals.begin(),edgei_map);

	for (auto v : rotations){
		for(auto e : v){
			std::cout << e << ' ' <<  get(edge_signals_map,e);
		}
		std::cout << std::endl;
	}

}
//TODO unit tests for dpc functions
//TODO test folder
//TODO function planar double cover -> planar polygon of graph
//TODO make this into a program
//TODO make a program to get an embedding

int main(){

}
