#pragma once

#include <string>

#include <boost/graph/graphviz.hpp>

#include <gdraw/graph_types.hpp>

/*
 * Reads a DOT files from stdin and returns the graph.
 */

namespace gdraw{

template <typename Graph>
Graph readDOT() noexcept
{

	Graph g = {};
	boost::dynamic_properties dp (boost::ignore_other_properties);

	bool status = read_graphviz(std::cin,g,dp);

	if(!status){
		std::cout << "Error while loading graph from stdin" << std::endl;
	}

	//Initializes the edge index map. The vertex one is automatically initialized
	//according to the insertion order of the vertices
	auto edgei_map = get( boost::edge_index, g);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(ei = edges(g).first; ei!=edges(g).second; ++ei)
		put(edgei_map,*ei,ecount++);

	return g;
}



template <typename Graph>
std::map<edge_t<Graph>,cubicSpline> getEdgeCoordinates(const Graph& g,
	       	const Graph& gp,
	       	const rotations_t<Graph>& rotations,
	       	const std::vector<coord_t>& coordinates ) noexcept{
	auto ognvertices = num_vertices(g);

	//TODO change to a unordered_map using a simple hash function on a pair of integers
	std::map<edge_t<Graph>,cubicSpline> xcoordinates;

	for(auto i = ognvertices; i < rotations.size() ; i++){
		auto ri = rotations.at(i);

		auto e1 = target(ri.at(0),gp) != i? target(ri.at(0),gp) : source(ri.at(0),gp);
		auto e2 = target(ri.at(2),gp) != i? target(ri.at(2),gp) : source(ri.at(2),gp);

		auto f1 = target(ri.at(1),gp) != i? target(ri.at(1),gp) : source(ri.at(1),gp);
		auto f2 = target(ri.at(3),gp) != i? target(ri.at(3),gp) : source(ri.at(3),gp);

		auto e = edge(e1,e2,g).first;
		auto f = edge(f1,f2,g).first;
	//	std::cout << "Crossing: " << e << " and " << f << std::endl;
	//	std::cout << "Coordinates: " << coordinates.at(i) << std::endl;
		cubicSpline espline = crossingSpline(coordinates.at(i),std::make_pair(coordinates.at(e1),coordinates.at(e2)));
	//	std::cout << "espline" << std::endl;
	//	std::cout << espline << std::endl;
		cubicSpline fspline = crossingSpline(coordinates.at(i),std::make_pair(coordinates.at(f1),coordinates.at(f2)));
	//	std::cout << "fspline" << std::endl;
	//	std::cout << fspline << std::endl;
		xcoordinates.insert(std::make_pair(e,espline)); 
		xcoordinates.insert(std::make_pair(f,fspline));
	}

	//TODO edges at xcoordinates do not exist in the modified graph
	return xcoordinates;
}

template <typename Graph>
void writeDOT(const DrawnGraph<Graph>& g, std::ostream& out=std::cout){
	auto vertex_pos_writer = [&g](auto&& out,auto&& v){
		out << "pos=\"" << g.coordinates[v] << "\"";
	};
	auto edge_pos_writer = [&g](auto&& out,auto&& e){
		if(g.edge_coordinates.contains(e))
			out << "pos=\"" << g.edge_coordinates.at(e) << "\"";
	};
	auto edge_color_writer = [&g](auto&& out,auto&& e){
		if(g.edge_colors.contains(e)){
			out << "color=" << g.edge_colors.at(e);
		}
	};
	auto vertex_color_writer = [&g](auto&& out,auto&& v){
		if(!g.vertex_colors.empty() && g.vertex_colors[v]!=""){
			out << "color=" << g.vertex_colors[v];
		}
	};
	auto vertex_writer = [&vertex_pos_writer,&vertex_color_writer](auto&& out,auto&& v){
		std::ostringstream pos;
		std::ostringstream color;
		vertex_pos_writer(pos,v);
		vertex_color_writer(color,v);
		//std::cout << "Pos: " << pos.str() << std::endl;
		//std::cout << "color: " << color.str() << std::endl;
		if(pos.str()!="" || color.str()!=""){
			out << '[';
			out << pos.str();
			if(color.str()!="" && pos.str()!="")
				out << ',';
			out << color.str();
			out << ']';
		}
	};
	auto edge_writer = [&edge_pos_writer,&edge_color_writer](auto&& out,auto&& e){
		std::ostringstream pos;
		std::ostringstream color;
		edge_pos_writer(pos,e);
		edge_color_writer(color,e);
		if(pos.str()!="" || color.str()!=""){
			out << '[';
			out << pos.str();
			if(color.str()!="" && pos.str()!="")
				out << ',';
			out << color.str();
			out << ']';
		}
	};
	boost::write_graphviz(out,g.getGraph(),vertex_writer,edge_writer);
}

template<template<typename> typename Wrapper,typename Graph>
requires AsGraphWrapper<Wrapper,Graph>
void writeDOT(const Wrapper<Graph> g,std::ostream& out=std::cout){
	boost::write_graphviz(out,g.getGraph());
}


}
