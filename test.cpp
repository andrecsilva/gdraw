#include "pplane.hpp"
#include "util.hpp"

#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 


int main(){


	AdjList g = getKn<AdjList>(5);
	//AdjList g = getKpq<AdjList>(5,5);
	//AdjList g{8};
	//

	//add_edge(1,0,g);
	//add_edge(2,1,g);
	//add_edge(2,3,g);
	//add_edge(3,0,g);

	//add_edge(0,4,g);
	//add_edge(1,5,g);
	//add_edge(2,6,g);
	//add_edge(3,7,g);

	//Tree<AdjList> tree;
	//tree = dfsTree(g,vertex(0,g));

	embedding_t embedding = readEmbedding();	

	//for(size_t i=0; i < embedding.size(); i++){
	//	std::cout << i << ":" << std::endl;
	//	for (size_t j=0; j< embedding.at(i).size(); j++){
	//		auto p = embedding.at(i).at(j);
	//		std::cout << p.first << " " << p.second << "\t";
	//	}
	//	std::cout << std::endl;
	//}

	//printGraph(g);
	std::vector<int> esignals = getEdgeSignals(g,embedding);

	for(size_t i =0 ; i< esignals.size(); i++)
		std::cout << "[" << i << "]" << esignals.at(i) << " ";
	std::cout << std::endl;

	AdjList h = planarDoubleCover(g,esignals);
	
	printGraph(h);
}
