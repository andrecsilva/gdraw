#include "io.hpp"
#include "util.hpp"
#include "xnumber.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,int>
	,boost::property<boost::edge_index_t,int>
	>; 

int main(){
	
	int k;
	std::cin >> k;
	AdjList g = readDOT<AdjList>();
	AdjList gp = AdjList(g);

	rotations_t<AdjList> rotations;

	bool answer =
	       	leqXnumberk(gp,rotations,k);

	if(answer){
		auto coordinates = draw(gp);
		writeDOT(std::cout,g,coordinates,{},getEdgeCoordinates(g,gp,rotations,coordinates));
	}

	return 0;

}
