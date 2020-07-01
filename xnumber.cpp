#include "include/gdraw/io.hpp"
#include "include/gdraw/draw.hpp"
#include "include/gdraw/xnumber.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,int>
	,boost::property<boost::edge_index_t,int>
	>; 

int main(int argc, char *argv[]){

	if(argc != 2)	{
		std::cout << "Usage: ./xnumber <k> < <graph>" << std::endl; 
		std::cout << "Where <k> is the queried crossing number and <graph> is the DOT format graph file." << std::endl;
		std::cout << "If the crossing number of <graph> is <= <k> the output will be a graph in DOT format with the drawing." << std::endl;
		return 0;
	}
	int k = atoi(argv[1]);
	//std::cin >> k;
	AdjList g = gdraw::readDOT<AdjList>();
	AdjList gp = AdjList(g);

	rotations_t<AdjList> rotations;

	bool answer =
	       	gdraw::leqXnumberk(gp,rotations,k);

	if(answer){
		gdraw::makeMaximalPlanar(gp);

		auto cycle = gdraw::findFacialCycle(gp);

		auto coordinates = gdraw::tutteDraw(gp,cycle);
		gdraw::writeDOT(std::cout,g,coordinates,{},gdraw::getEdgeCoordinates(g,gp,rotations,coordinates));
	}

	return 0;

}
