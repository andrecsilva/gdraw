#ifndef DRAW_HPP
#include <string>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>

#include "coordinates.hpp"

#include "graph_types.hpp"

namespace gdraw{

template <typename Graph>
void makeMaximalPlanar(Graph& g){

	auto edgei_map = get( boost::edge_index, g);
	typename boost::graph_traits<Graph>::edges_size_type ecount = num_edges(g);
	typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;


	//Find the original edge with index 0.
	//We want the newly added edges with index >= num_edges(g)
	edge_t<Graph> original_0_edge;
	for(boost::tie(ei,ei_end) = edges(g);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0)
			original_0_edge = *ei;
			
	rotations_t<Graph> embedding(num_vertices(g));
	//typedef std::vector< typename boost::graph_traits<Graph>::edge_descriptor > vec_t;
	//std::vector<vec_t> embedding(num_vertices(g));
	

	//make biconnected
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	//std::cout << "Rotations:" << std::endl;

	//for(auto i = embedding.begin(); i!=embedding.end();i++){
	//	std::cout << std::distance(embedding.begin(),i) << std::endl;
	//	for(auto ei = i->begin(); ei!=i->end();ei++)	
	//		std::cout << *ei << " ";
	//std::cout << std::endl;
	//}

	make_biconnected_planar(g,&embedding[0]);

	//std::cout << "Make Biconnected" << std::endl;
	//printGraph(g);

	//Add edge_index for the newly added edges
	for(boost::tie(ei,ei_end) = edges(g);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0 && *ei!=original_0_edge)
			put(edgei_map,*ei,ecount++);


	//make maximal
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	//TODO see what happens if the embedding here is reused... boost example passes &embedding[0]...
	make_maximal_planar(g,&embedding[0]);

	//std::cout << "Make Maximal" << std::endl;
	//printGraph(g);
	//Add edge_index for the newly added edges
	for(boost::tie(ei,ei_end) = edges(g);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0 && *ei!=original_0_edge)
			put(edgei_map,*ei,ecount++);

	//printGraph(g);
}

/*
 * Uses the algorithm of Chrobak-Payne to draw a planar graph.
 */
template <typename Graph>
std::vector<coord_t> chrobakPayneDraw(const Graph& g) noexcept{

	//copy graph
	Graph g_maximal= Graph{g};

	makeMaximalPlanar(g_maximal);

	rotations_t<Graph> embedding(num_vertices(g));
	//canonical ordering	
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g_maximal
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	std::vector<vertex_t<Graph> > ordering;
	planar_canonical_ordering(g_maximal, &embedding[0], std::back_inserter(ordering));

	
	//get drawing
	
	std::vector<coord_t> coordinates(num_vertices(g_maximal));

	chrobak_payne_straight_line_drawing(g_maximal,
			  embedding, 
			  ordering.begin(),
			  ordering.end(),
			  &coordinates[0]);

	//printGraph(g_maximal);

	return coordinates;
}


//void findBridges(Graph& G, Tree& T, edge_t<Graph>& e);
//if there is a path from v to T+e then v is a bridge
//if the edge f joins two vertices of T+e then f is a trivial bridge

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

/*
 * Solves a system of linear equations of the form Ax=b.
 * The vector b is modified and contains the solution.
 */
void
solveSystem(boost::numeric::ublas::matrix<double>& A,
	       	boost::numeric::ublas::vector<double>& b){

	boost::numeric::ublas::permutation_matrix<size_t> pm(A.size1());
	lu_factorize(A,pm);

	lu_substitute(A,pm,b);
}

template <typename Vertex>
struct FacialCyclesVisitor : public boost::planar_face_traversal_visitor{

	FacialCyclesVisitor(std::vector<std::vector<Vertex>>& facial_cycles) : facial_cycles(&facial_cycles) {};

	std::vector<std::vector<Vertex>>* facial_cycles;

	std::vector<Vertex> current_cycle;

	void begin_face(){
		current_cycle = {};	
		//std::cout << "New Face: " << std::endl;
	}

	void next_vertex(Vertex v){
		current_cycle.push_back(v);
		//std::cout << v << " ";
	}

	void end_face(){
		facial_cycles->push_back(std::move(current_cycle));
		//std::cout << std::endl;
	}

};


//TODO do one for 3-connected graphs
/*
 * Assumes G is planar and connected
 */
template <typename Graph>
std::vector<vertex_t<Graph>> findFacialCycle(const Graph& g){
	std::vector<std::vector<vertex_t<Graph>>> facial_cycles;
	FacialCyclesVisitor<vertex_t<Graph>> fcv {facial_cycles};


	rotations_t<Graph> embedding(num_vertices(g));
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	//for(auto v : embedding){
	//	for(auto w : v)
	//		std::cout << w << " ";
	//	std::cout << std::endl;
	//}

	planar_face_traversal(g,&embedding[0],fcv);
	return facial_cycles[0];
}


/*
 *Tutte's Algorithm from "How to Draw a Graph".
 */
template <typename Graph>
std::vector<coord_t> tutteDraw(const Graph& g, const std::vector<vertex_t<Graph>>& facial_cycle){


	//copy graph
	Graph g_maximal= Graph{g};

	makeMaximalPlanar(g_maximal);

	std::vector<coord_t> coordinates(num_vertices(g_maximal));
	std::vector<std::pair<vertex_t<Graph>,coord_t>> polygon = cycleToPolygon<Graph>(facial_cycle,num_vertices(g_maximal));
	std::vector<bool> in_polygon(num_vertices(g_maximal),false);

	for (auto p : polygon){
		coordinates.at(p.first) = p.second;
		in_polygon.at(p.first) = true;
	}


	auto [A,bx,by,vertex_to_row] = buildSystem(g_maximal,polygon);

	//std::cout << A << std::endl;
	//for(auto v: vertex_to_row)
	//	std::cout << v <<",";
	//std::cout << std::endl;

	solveSystem(A,bx);
	solveSystem(A,by);

	//std::cout << bx << std::endl;
	//std::cout << by << std::endl;

	for(size_t i =0; i< vertex_to_row.size();i++)
		if(!in_polygon[i])
			coordinates.at(i) = {bx(vertex_to_row[i]),by(vertex_to_row[i])};


	return coordinates;
}

} //namespace

#endif
