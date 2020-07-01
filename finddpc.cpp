#include <iostream>

#include <string>
#include "pplane.hpp"
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



	AdjList g = readDOT<AdjList>();

	auto [found,dpc] = findDoublePlanarCover(g);


	if(found){
		rotations_t<AdjList> rotations;
		std::vector<int> edge_signals;
		std::tie(rotations,edge_signals) = embeddingFromDPC(g,dpc);
		auto xedges = getCrossEdges(dpc,num_vertices(g));

		std::map<edge_t<AdjList>,std::string> edge_color;

		for(auto e : xedges)
			edge_color[e] = "red";

		AdjList dpc_copy {dpc};
		makeMaximalPlanar(dpc_copy);

		auto cycle = findFacialCycle(dpc_copy);

		auto coordinates = tutteDraw(dpc_copy,cycle);

		writeDOT(std::cout,dpc,coordinates,{},{},edge_color);
	}else
		std::cout << "No double planar cover found";


}
