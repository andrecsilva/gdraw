#include <iostream>

#include <boost/graph/adjacency_list.hpp>

#include "include/gdraw/pplane.hpp"
#include "include/gdraw/io.hpp"

using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 

template <typename Graph>
void printEmbedding(const Graph& g,std::tuple<rotations_t<Graph>,std::vector<int>> embedding){

	auto [rotations,edge_signals] = embedding;

	auto edgei_map = boost::get(boost::edge_index,g);
	auto edge_signals_map =  boost::make_iterator_property_map(edge_signals.begin(),edgei_map);

	for (size_t i =0; i<rotations.size(); i++){
		for(auto e : rotations[i]){
			auto v = source(e,g) == i ? target(e,g) : source(e,g); 
			std::cout << v << ' ' <<  get(edge_signals_map,e) << '\t';
		}
		std::cout << std::endl;
	}

}

int main(){

	AdjList g = gdraw::readDOT<AdjList>();

	//gdraw::printGraph(g);

	rotations_t<AdjList> rotations;
	std::vector<int> edge_signals;

	bool found = gdraw::findProjectiveEmbedding(g,std::tie(rotations,edge_signals));

	if(found)
		printEmbedding(g,std::make_tuple(rotations,edge_signals));
	else
		std::cout << "No embeddings found." << std::endl;

}
