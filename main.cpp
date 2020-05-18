#include "util.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>


int main(){
	
	int k;
	std::cin >> k;
	Graph g = readDOT();
	Graph gp = Graph(g);

	rotations_t<Graph> rotations;

	bool answer =
	       	leqXnumberk(gp,rotations,k);

	if(answer){
		std::vector<coord_t> coordinates {num_vertices(gp)};
		coordinates = draw(gp);
		writeDOT(std::cout,g,gp,rotations,coordinates);
	}

	return 0;

}
