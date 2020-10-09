#include <iostream>
#include <string>

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

	auto g = GraphWrapper{readDOT<AdjList>()};

	auto n = num_vertices(g.getGraph());

	auto result = findDoublePlanarCover(std::move(g));

	if(result){
		auto dg = tutteDraw(std::move(result.value()));

		std::vector<edge_t<AdjList>> xedges;

		auto is_x_edge = [&dg,&n](auto&& e){
			auto [u,v] = endpoints(dg.getGraph(),e);
			return (u < n && v >=n) || (u >=n && v < n);
		};

		for(auto&& e : range(edges(dg.getGraph())) | std::views::filter(is_x_edge))
			dg.colorEdge(e,"red");
		
		writeDOT(dg);
	}
}
