#include <iostream>
#include <string>

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

std::vector<coord_t> tutteDraw();

template <typename Graph>
std::tuple<boost::numeric::ublas::matrix<double>,
	boost::numeric::ublas::vector<double>,
	boost::numeric::ublas::vector<double>> 
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
	return {A,bx,by};
}

void
solve(boost::numeric::ublas::matrix<double>& A,
	       	boost::numeric::ublas::vector<double>& b){

	boost::numeric::ublas::permutation_matrix<size_t> pm(A.size1());
	lu_factorize(A,pm);

	lu_substitute(A,pm,b);
}

std::vector<coord_t> baricenters();

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

	add_edge(2,2,g);
	add_edge(2,3,g);
	add_edge(2,4,g);

	add_edge(3,4,g);
	std::vector<std::pair<vertex_t<AdjList>,coord_t>> polygon = { {0,{3,6}}, {1,{0,3}}, {2,{4,1}} };
	auto [A,bx,by] = buildSystem(g,polygon);

	std::cout << A << std::endl;
	std::cout << bx << std::endl;
	std::cout << by << std::endl;

	solve(A,bx);
	solve(A,by);

	std::cout << bx << std::endl;
	std::cout << by << std::endl;
}
