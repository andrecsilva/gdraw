#include <iostream>
#include <string>
#include <math.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>


#include "pplane.hpp"
#include "util.hpp"
#include "io.hpp"
#include "draw.hpp"

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 



int main(){


	//AdjList g = getKn<AdjList>(6);
	//AdjList g = getKpq<AdjList>(5,5);
	AdjList g = getV8<AdjList>();


	//Tree<AdjList> tree;
	//tree = dfsTree(g,vertex(0,g));

	//embedding_t embedding = readEmbedding();	

	////for(size_t i=0; i < embedding.size(); i++){
	////	std::cout << i << ":" << std::endl;
	////	for (size_t j=0; j< embedding.at(i).size(); j++){
	////		auto p = embedding.at(i).at(j);
	////		std::cout << p.first << " " << p.second << "\t";
	////	}
	////	std::cout << std::endl;
	////}

	////printGraph(g);
	//std::vector<int> esignals = getEdgeSignals(g,embedding);

//	//for(size_t i =0 ; i< esignals.size(); i++)
//	//	std::cout << "[" << i << "]" << esignals.at(i) << " ";
//	//std::cout << std::endl;

	//AdjList h = planarDoubleCover(g,esignals);
	//
	////printGraph(h);

	//auto gn = num_vertices(g);

	//std::map<edge_t<AdjList>,std::string> edge_colors;

	//for(auto [ei,ei_end] = edges(h);ei!=ei_end;ei++)
	//	if((target(*ei,h) < gn && source(*ei,h) >=gn) ||
	//		(source(*ei,h) < gn && target(*ei,h) >=gn))
	//		edge_colors[*ei]="red";

	////writeDOT(std::cout,h,draw(h),{},{},edge_colors);
	//
	//std::vector<vertex_t<AdjList>> cycle = findFacialCycle(h);
	//
	////std::cout << "Cycle: " <<  std::endl;
	////for (auto v : cycle)
	////	std::cout << v << " ";
	////std::cout << std::endl;

	//auto coordinates = tutteDraw(h,cycle);

	////for(auto c: coordinates)
	////	std::cout << c << std::endl;

	//writeDOT(std::cout,h,coordinates,{},{},edge_colors);

	////std::cout << bx << std::endl;
	////std::cout << by << std::endl;
	
	std::vector<edge_t<AdjList>> xedges = {
		edge(0,4,g).first,
		edge(1,5,g).first,
		edge(2,6,g).first,
		edge(3,7,g).first,
	};

	AdjList h = doubleCover(g,xedges);

	auto xedges_2 = getCrossEdges(h,num_vertices(g));

	//for(auto e : xedges_2)
	//	std::cout << e << std::endl;
	
	auto [rotations,edge_signal] =  embeddingFromDPC(g,h);

	for(auto u : rotations){
		for(auto e : u)
			std::cout << e << " ";
		std::cout << std::endl;
	}

	auto edgei_map = get(boost::edge_index,g);
	edge_signals_t<AdjList> edge_signal_map (edge_signal.begin(),edgei_map);

	for(auto [ei,ei_end] = edges(g); ei!=ei_end; ++ei){
		std::cout << "[" << boost::get(edgei_map,*ei) << "]" << *ei << " " << get(edge_signal_map,*ei) << " ";
	}
	std::cout << std::endl;


	printGraph(g);
}
