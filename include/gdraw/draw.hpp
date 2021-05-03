#pragma once
#include <string>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>

#include <gdraw/graph_types.hpp>
#include <gdraw/util.hpp>
#include <gdraw/planar_graphs.hpp>

namespace gdraw{

template <typename Graph,typename Range>
requires VertexRange<Range,Graph>
auto drawCycle(const Range& cycle,const double radius=10){

	std::vector<std::pair<vertex_t<Graph>,coord_t>> polygon;
	double step = 2 * M_PI / (std::ranges::size(cycle));
	for(auto i =0; auto&& v : cycle){
		coord_t coord = {radius*cos(i*step), radius*sin(i*step)};
		polygon.push_back({v, coord});
		i++;
	}

	//std::cout << "Step: " << step << std::endl;
	//std::cout << "Step FULL: " << step * cycle.size() << std::endl;
	//for(auto p : polygon){
	//	std::cout << p.first << " : " << p.second << std::endl;
	//}
	
	return polygon;
}

//TODO Use armadillo instead of boost::numeric::ublas?
//TODO std::allocator::construct is depreceated in c++20, need to change boost's code...
template <typename Graph,typename DrawnCycle>
auto buildSystem(const Graph& g,
	       	const DrawnCycle& polygon){

	std::vector<size_t> vertex_to_row(num_vertices(g));
	size_t size = num_vertices(g) - polygon.size();

	boost::numeric::ublas::matrix<double> A(size,size);
	boost::numeric::ublas::vector<double> bx(size);
	boost::numeric::ublas::vector<double> by(size);

	std::vector<std::pair<bool,coord_t>> in_polygon(num_vertices(g),std::make_pair(false,coord_t{0,0}));
	//std::cout << "In Polygon: " << std::endl;
	for(auto&& [u,coord] : polygon){
		in_polygon[u].first = true;
		in_polygon[u].second = coord;
		//std::cout << u << " " << coord << std::endl;
	}

	int row=0;
	for (auto&& v : range(vertices(g)))
		if(!in_polygon[v].first)
			vertex_to_row[v]=row++;


	for (auto&& v : range(vertices(g))){
		//std::cout << "Vertex: " << v << std::endl;
		if(!in_polygon[v].first){
			A(vertex_to_row[v],vertex_to_row[v]) = out_degree(v,g);
			//std::cout << "To Row: " << vertex_to_row[v] << std::endl;
			//std::cout << A << std::endl;
			for(auto&& e : range(out_edges(v,g))){
				auto u = target(e,g);
				//std::cout << "target: " << target(*ei,g) << std::endl;
				//std::cout << "source: " << u << std::endl;
				if(in_polygon[u].first){
					bx[vertex_to_row[v]] += in_polygon[u].second.x;
					by[vertex_to_row[v]] += in_polygon[u].second.y;
				}else{
					A(vertex_to_row[v],vertex_to_row[u])=-1;

				}
			}
		}
	}
	return std::make_tuple(A,bx,by,vertex_to_row);
}

/**
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

/**
 * Draws a given planar graph using the algorithm by W. Tutte in "How to Draw a Graph".
 *
 * @param g : A embedded planar graph.
 * @param facial_cycle: A sequence (i.e. std::range) of vertices that induces a cyle in `g`.
 */
template <typename Graph,typename Range>
requires VertexRange<Range,Graph>
auto tutteDraw(PlanarGraph<Graph> g, const Range& facial_cycle) -> DrawnGraph<Graph>{

	//copy graph here, we don't want the extra edges...
	auto g_maximal = makeMaximal(g);

	std::vector<coord_t> coordinates(num_vertices(g_maximal.getGraph()));

	auto drawn_cycle = drawCycle<Graph>(facial_cycle);

	std::vector<bool> in_cycle(num_vertices(g_maximal.getGraph()),false);

	for(auto p : drawn_cycle){
		coordinates[p.first] = p.second;
		in_cycle[p.first] = true;
	}

	auto [A,bx,by,vertex_to_row] = buildSystem(g.getGraph(),drawn_cycle);
	
	solveSystem(A,bx);
	solveSystem(A,by);

	auto not_in_cycle = [&in_cycle](auto&& p){
		return not in_cycle[p];
	};

	//TODO would like a better solution to this, without using enumerate
	for(size_t v=0;v!=vertex_to_row.size();v++){
		auto i = vertex_to_row[v];
		if(not_in_cycle(v))
			coordinates[v] = {bx(i),by(i)};
	}

	return DrawnGraph<Graph>(std::move(g),coordinates);
}

/**
 * Draws a planar graph using the algorithm by W. Tutte in "How to Draw a Graph".
 *
 * Works just like tutteDraw(g,facial_cycle) but this one finds a cycle for you.
 */
template <typename Graph>
auto tutteDraw(PlanarGraph<Graph> g) -> DrawnGraph<Graph>{
	auto facial_cycle = findFacialCycle(g);
	return tutteDraw(std::move(g),facial_cycle);
}


template <typename Graph>
auto chrobakPayneDraw(PlanarGraph<Graph>& g) -> DrawnGraph<Graph>{

	//copy graph here, we don't want the extra edges...
	auto g_maximal = makeMaximal(g);

	std::vector<vertex_t<Graph> > ordering;
	auto rotations_pmap = make_iterator_property_map(g_maximal.rotations.begin(),get(boost::vertex_index,g_maximal.getGraph()));
	planar_canonical_ordering(g_maximal.getGraph(), rotations_pmap, std::back_inserter(ordering));

	std::vector<coord_t> coordinates(num_vertices(g_maximal.getGraph()));

	auto coordinates_pmap = make_iterator_property_map(coordinates.begin(),get(boost::vertex_index,g_maximal.getGraph()));

	chrobak_payne_straight_line_drawing(g_maximal.getGraph(),
			  g_maximal.rotations,
			  ordering.begin(),
			  ordering.end(),
			  coordinates_pmap);

	return DrawnGraph<Graph>(std::move(g),coordinates);
}

inline auto crossingSpline(const coord_t& xpoint, const std::pair<coord_t,coord_t>& e, const float epsilon=0.2) -> cubicSpline {
	const coord_t p1 {xpoint.x + (e.first.x - xpoint.x ) * epsilon, xpoint.y +  (e.first.y - xpoint.y) * epsilon};
	//coord_t p2 {xpoint.x + (e.first.x - xpoint.x ) * epsilon/2, xpoint.y +  (e.first.y - xpoint.y) * epsilon/2};
	const coord_t p2 {xpoint.x + (e.second.x - xpoint.x ) * epsilon, xpoint.y +  (e.second.y - xpoint.y) * epsilon};
	cubicSpline spline { e.first, p1, p2, e.second};
	return spline;
}

template <typename Graph>
auto drawFlattenedGraph(PlanarGraph<Graph> g, size_t original_vcount, std::function<DrawnGraph<Graph>(PlanarGraph<Graph>)> draw_method = tutteDraw<Graph>) -> DrawnGraph<Graph>{

	auto rotations {g.rotations};

	std::map<edge_t<Graph>,cubicSpline> xcoordinates;

	auto dg = draw_method(std::move(g));


	auto other_endpoint = [&dg](auto&& u, auto&& e){
		return target(e,dg.getGraph()) != u ? target(e,dg.getGraph()) : source(e,dg.getGraph());
	};

	auto edgei_map = get(boost::edge_index, dg.getGraph());

	auto min_index = [&dg,&edgei_map](auto&& e, auto&& f){
		auto ei = boost::get(edgei_map,e);
		auto fi = boost::get(edgei_map,f);
		return std::min(ei,fi);
	};

	for(auto u = original_vcount; u < num_vertices(dg.getGraph()); u++){
		//std::cout << u << std::endl;
		auto pi_u = rotations[u];
		auto v = other_endpoint(u,pi_u[0]);
		auto w = other_endpoint(u,pi_u[2]);

		auto a = other_endpoint(u,pi_u[1]);
		auto b = other_endpoint(u,pi_u[3]);

		//auto e = edge(v,w,dg.getGraph()).first;
		//auto f = edge(a,b,dg.getGraph()).first;
		
		auto ei = min_index(pi_u[0],pi_u[2]);
		auto fi = min_index(pi_u[1],pi_u[3]);

		clear_vertex(u,dg.getGraph());

		auto e = add_edge(v,w,dg.getGraph()).first;
		auto f = add_edge(a,b,dg.getGraph()).first;

		boost::put(edgei_map,e,ei);
		boost::put(edgei_map,f,fi);
		
		auto espline = crossingSpline(dg.coordinates[u],std::make_pair(dg.coordinates[v],dg.coordinates[w]));
		auto fspline = crossingSpline(dg.coordinates[u],std::make_pair(dg.coordinates[a],dg.coordinates[b]));

		xcoordinates.insert({e,espline});
		xcoordinates.insert({f,fspline});
	}
	gdraw::removeIsolatedVertices(dg.getGraph());

	return DrawnGraph(std::move(dg), std::move(dg.coordinates), std::move(xcoordinates));;

}


} //namespace

