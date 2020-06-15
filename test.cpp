#include <iostream>
#include <string>
#include <math.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "pplane.hpp"
#include "util.hpp"
#include "io.hpp"

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

/*
 * Returns a set of cycle.size() points evenly spaced along the circle of the specifies radius (1 by default);
 * */
template <typename Graph>
std::vector<std::pair<vertex_t<Graph>,coord_t>> cycleToPolygon(const std::vector<vertex_t<Graph>>& cycle,const double radius=1){

	std::vector<std::pair<vertex_t<Graph>,coord_t>> polygon;
	double step = 2 * M_PI / (cycle.size());
	for (size_t i=0; i<cycle.size();i++){
		coord_t coord = {radius*cos(i*step), radius*sin(i*step)};
		polygon.push_back({ cycle.at(i), coord });
	}

	//std::cout << "Step: " << step << std::endl;
	//std::cout << "Step FULL: " << step * cycle.size() << std::endl;
	//for(auto p : polygon){
	//	std::cout << p.first << " : " << p.second << std::endl;
	//}
	
	return polygon;
}

template <typename Graph>
std::tuple<boost::numeric::ublas::matrix<double>,
	boost::numeric::ublas::vector<double>,
	boost::numeric::ublas::vector<double>,
	std::vector<size_t>>
	buildSystem(const Graph& g,
	       	const std::vector<std::pair<vertex_t<Graph>,coord_t>>& polygon){

	std::vector<size_t> vertex_to_row(num_vertices(g));
	size_t size = num_vertices(g) - polygon.size();

	boost::numeric::ublas::matrix<double> A(size,size);
	boost::numeric::ublas::vector<double> bx(size);
	boost::numeric::ublas::vector<double> by(size);

	std::vector<std::pair<bool,coord_t>> in_polygon(num_vertices(g),std::make_pair(false,coord_t{0,0}));
	//std::cout << "In Polygon: " << std::endl;
	for(auto [u,coord] : polygon){
		in_polygon[u].first = true;
		in_polygon[u].second = coord;
		//std::cout << u << " " << coord << std::endl;
	}

	int row=0;
	for (auto [vi,vi_end] = vertices(g); vi!=vi_end; vi++)
		if(!in_polygon[*vi].first)
			vertex_to_row[*vi]=row++;


	for (auto [vi,vi_end] = vertices(g); vi!=vi_end; vi++){
		//std::cout << "Vertex: " << *vi << std::endl;
		if(!in_polygon[*vi].first){
			A(vertex_to_row[*vi],vertex_to_row[*vi]) = out_degree(*vi,g);
			//std::cout << "To Row: " << vertex_to_row[*vi] << std::endl;
			//std::cout << A << std::endl;
			for(auto [ei,ei_end] = out_edges(*vi,g);ei!=ei_end;ei++){
				auto u = target(*ei,g);
				//std::cout << "target: " << target(*ei,g) << std::endl;
				//std::cout << "source: " << u << std::endl;
				if(in_polygon[u].first){
					bx[vertex_to_row[*vi]] += in_polygon[u].second.x;
					by[vertex_to_row[*vi]] += in_polygon[u].second.y;
				}else{
					A(vertex_to_row[*vi],vertex_to_row[u])=-1;

				}
			}
		}
	}
	return {A,bx,by,vertex_to_row};
}

void
solve(boost::numeric::ublas::matrix<double>& A,
	       	boost::numeric::ublas::vector<double>& b){

	boost::numeric::ublas::permutation_matrix<size_t> pm(A.size1());
	lu_factorize(A,pm);

	lu_substitute(A,pm,b);
}

template <typename Graph>
std::vector<coord_t> tutteDraw(const Graph& g, const std::vector<vertex_t<Graph>>& facial_cycle){

	std::vector<coord_t> coordinates(num_vertices(g));
	std::vector<std::pair<vertex_t<Graph>,coord_t>> polygon = cycleToPolygon<Graph>(facial_cycle,num_vertices(g));
	std::vector<bool> in_polygon(num_vertices(g),false);

	for (auto p : polygon){
		coordinates.at(p.first) = p.second;
		in_polygon.at(p.first) = true;
	}


	auto [A,bx,by,vertex_to_row] = buildSystem(g,polygon);

	std::cout << A << std::endl;
	std::cout << bx << std::endl;
	std::cout << by << std::endl;

	solve(A,bx);
	solve(A,by);

	for(size_t i =0; i< vertex_to_row.size();i++)
		if(!in_polygon[i])
			coordinates.at(i) = {bx(vertex_to_row[i]),by(vertex_to_row[i])};


	return coordinates;
}


int main(){


	//AdjList g = getKn<AdjList>(6);
	//AdjList g = getKpq<AdjList>(5,5);
	//AdjList g{8};
	//

	//add_edge(1,0,g);
	//add_edge(2,1,g);
	//add_edge(2,3,g);
	//add_edge(3,0,g);

	//add_edge(0,4,g);
	//add_edge(1,5,g);
	//add_edge(2,6,g);
	//add_edge(3,7,g);

	//Tree<AdjList> tree;
	//tree = dfsTree(g,vertex(0,g));

	//embedding_t embedding = readEmbedding();	

	//for(size_t i=0; i < embedding.size(); i++){
	//	std::cout << i << ":" << std::endl;
	//	for (size_t j=0; j< embedding.at(i).size(); j++){
	//		auto p = embedding.at(i).at(j);
	//		std::cout << p.first << " " << p.second << "\t";
	//	}
	//	std::cout << std::endl;
	//}

	//printGraph(g);
	//std::vector<int> esignals = getEdgeSignals(g,embedding);

//	//for(size_t i =0 ; i< esignals.size(); i++)
//	//	std::cout << "[" << i << "]" << esignals.at(i) << " ";
//	//std::cout << std::endl;

	//AdjList h = planarDoubleCover(g,esignals);
	//
	////printGraph(h);

	//auto gn = num_vertices(g);

	//std::map<edge_t<AdjList>,std::string> edge_colors;

	//for(auto [ei,ei_end] = edges(h);ei!=ei_end;ei++)
	//	if((target(*ei,h) < gn && source(*ei,h) >=gn) ||
	//		(source(*ei,h) < gn && target(*ei,h) >=gn))
	//		edge_colors[*ei]="red";

	//writeDOT(std::cout,h,draw(h),{},{},edge_colors);
	
	AdjList g(5);

	add_edge(0,1,g);
	add_edge(0,2,g);
	add_edge(0,3,g);

	add_edge(1,2,g);
	add_edge(1,3,g);
	add_edge(1,4,g);

	add_edge(2,3,g);
	add_edge(2,4,g);

	add_edge(3,4,g);

	auto coordinates = tutteDraw(g,{0,1,2});

	writeDOT(std::cout,g,coordinates);

	//std::cout << bx << std::endl;
	//std::cout << by << std::endl;
}
