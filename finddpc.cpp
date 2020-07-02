#include <iostream>
#include <string>

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

int main(){



	AdjList g = gdraw::readDOT<AdjList>();

	auto [found,dpc] = gdraw::findDoublePlanarCover(g);


	if(found){
		rotations_t<AdjList> rotations;
		std::vector<int> edge_signals;
		std::tie(rotations,edge_signals) = gdraw::embeddingFromDPC(g,dpc);
		auto xedges = gdraw::getCrossEdges(dpc,num_vertices(g));

		std::map<edge_t<AdjList>,std::string> edge_color;

		for(auto e : xedges)
			edge_color[e] = "red";

		AdjList dpc_copy {dpc};
		gdraw::makeMaximalPlanar(dpc_copy);

		auto cycle = gdraw::findFacialCycle(dpc_copy);

		auto coordinates = gdraw::tutteDraw(dpc_copy,cycle);

		gdraw::writeDOT(std::cout,dpc,coordinates,{},{},edge_color);
	}else
		std::cout << "No double planar cover found" << std::endl;


}
