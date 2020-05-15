#include "util.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>


int main(){

	//Graph g = readDOT();
	//Graph g = getKpq(3,4);
	Graph g = getKn(6);
	Graph gp = Graph(g);

	printGraph(gp);
//
	rotations_t<Graph> rotations;
//
//	//if(isPlanar(g,kuratowski_edges, rotations))
//	//	return false;
//
	int k = 3;
	bool answer =
	       leqXnumberk(gp,rotations,k);

	//auto edgei_map = get( boost::edge_index, gp);
//
	removeIsolatedVertices(gp);

	printGraph(gp);

	//auto edgei_map = get( boost::edge_index, gp);
	//typename boost::graph_traits<Graph>::edges_size_type ecount = 0;
	//typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
	//for(boost::tie(ei,ei_end) = edges(gp);ei!=ei_end;ei++)
	//	put(edgei_map,*ei,ecount++);

	std::cout << "cr(G) <= " <<  k <<  " ? :" << answer << std::endl;

	//std::cout << std::endl;
	std::cout << "Rotations:" << std::endl;

	for(auto i : rotations){
		for(auto e : i)	
			std::cout << e << " ";
	std::cout << std::endl;
	}

	//std::cout << "Before Pw"  << std::endl;
	//if (answer){
	//	std::vector<coord_t> coordinates {num_vertices(gp)};
	//	coordinates = draw(gp);
	//	writeDOT(std::cout,g,gp,rotations,coordinates);
	//}



	//std::cout << "Coordinates: " << std::endl;
//	typename boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
//	for(boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; vi++)
//		//std::cout << "[" << *vi << "]" << "(" << coordinates.at(*vi).x << "," << coordinates.at(*vi).y << ")";
//		std::cout << coordinates.at(*vi).x << " " << coordinates.at(*vi).y << "\t";
//	std::cout << std::endl;

	//printGraph(g);

	//isPlanar(g);
	
	//int k = 1;

	//add_vertex(g);

	//auto edgei_map = get( boost::edge_index, g);

	//auto ei = boost::add_edge(5,0,g);
	//
	//typename boost::graph_traits<Graph>::edges_size_type ecount = 9;

	//put(edgei_map,ei,ecount);

	//printGraph(g);

	return 0;

}
