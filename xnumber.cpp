#include <gdraw/io.hpp>
#include <gdraw/draw.hpp>
#include <gdraw/xnumber.hpp>

#include <boost/graph/adjacency_list.hpp>

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
	auto g = gdraw::GraphWrapper<AdjList>{gdraw::readDOT<AdjList>()};

	auto n = num_vertices(g.getGraph());

	auto result = gdraw::planarXNumber(std::move(g),k);

	if(result){
		auto dg = gdraw::drawFlattenedGraph(std::move(result.value()),n);
		gdraw::writeDOT(dg);
	}

	return 0;

}
