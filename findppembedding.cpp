#include <iostream>

#include <boost/graph/adjacency_list.hpp>

#include <gdraw/pplane.hpp>
#include <gdraw/io.hpp>
#include <gdraw/draw.hpp>

using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 

int main(){
	using namespace gdraw;

	auto g = IndexedGraph{readDOT<AdjList>()};

	auto result = findDoublePlanarCover(std::move(g));

	if(result){
		auto ppg = embeddingFromDPC(std::move(result.value()));
		std::vector<edge_t<AdjList>> xedges;
		printEmbedding(ppg);
	}
}
