#pragma once
#include <string>
#include <armadillo>

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

/**
 * Returns the Laplacian Matrix of a graph g.
 */
template <typename Graph>
auto laplacian(const IndexedGraph<Graph>& g){

	arma::mat D(num_vertices(g.getGraph()),num_vertices(g.getGraph()),arma::fill::zeros);

	for(size_t i=0; i<num_vertices(g.getGraph()); i++){
		D.diag()(i) = out_degree(vertex(i,g.getGraph()),g.getGraph());
	}

	for(auto&& e : range(edges(g.getGraph()))){
		auto [u,v] = endpoints(g.getGraph(),e);
		auto [i,j] = std::make_tuple(g.index(u),g.index(v));
		D(i,j) = -1;
		D(j,i) = D(u,v);
	}

	return D;
}

/**
 * Returns an list of vertices not in the cycle ordered by its indices.
 */
template <typename Graph>
auto not_in_cycle(const IndexedGraph<Graph>& g, const std::vector<vertex_t<Graph>>& cycle){

	std::vector<bool> in_cycle(num_vertices(g.getGraph()),false);
	for(auto&& v : cycle)
		in_cycle[g.index(v)]=true;

	//for(size_t i =0; i< in_cycle.size(); i++)
	//	std::cout << in_cycle[i] << ' ';
	//std::cout << std::endl;

	std::vector<vertex_t<Graph>> not_in_cycle;

	for(size_t i=0;i<num_vertices(g.getGraph()); i++)
		if(!in_cycle[i])
			not_in_cycle.push_back(vertex(i,g.getGraph()));

	return not_in_cycle;
}

/** Draws a graph according to Tutte's algorithm in 
 * "How to draw a graph". Returns a vector containing
 * containing the coordinates of g's vertices.
 */
template <typename Graph>
auto tutteDrawImpl(IndexedGraph<Graph>& g,
	       	std::vector<vertex_t<Graph>>& cycle,
	       	std::vector<coord_t>& cycle_coordinates
	       	){
	auto L = laplacian(g);

	arma::vec cx(g.numVertices(),arma::fill::zeros);
	arma::vec cy(g.numVertices(),arma::fill::zeros);

	for(size_t i=0; i<cycle_coordinates.size(); i++){
		cx(cycle[i]) = cycle_coordinates[i].x ;
		cy(cycle[i]) = cycle_coordinates[i].y ;

	}

	auto ncycle = not_in_cycle(g,cycle);
	arma::uvec idx(ncycle.size());

	for(size_t i =0; i< ncycle.size(); i++)
		idx(i)=ncycle[i];

	arma::uvec zero(1);
	zero(0) = 0;

	//strikeout the indices not in idx and invert it
	arma::mat Li = arma::inv_sympd(L(idx,idx));

	arma::mat bx = L*(-1 * cx);
	bx = bx.submat(idx,zero);
	arma::mat by = L*(-1 * cy);
	by = by.submat(idx,zero);

	arma::vec sbx = Li * bx;
	arma::vec sby = Li * by;

	arma::vec scx(g.numVertices(),arma::fill::zeros);
	arma::vec scy(g.numVertices(),arma::fill::zeros);

	for(size_t i = 0; i<ncycle.size(); i++){
		scx(ncycle[i]) = sbx(i);
		scy(ncycle[i]) = sby(i);
	}

	scx = cx + scx;
	scy = cy + scy;

	std::vector<coord_t> coordinates(g.numVertices());
	for(size_t i =0; i<g.numVertices(); i++){
		coordinates[i] = {scx(i),scy(i)};
	}

	return coordinates;
}

/**
 * Returns the coordinates of a convex polygon with 
 * cycle_size vertices and center (0,0).
 */
auto drawCycle(size_t cycle_size,const double radius=1){

	std::vector<coord_t> cycle_coordinates;
	double step = 2 * M_PI / (cycle_size);
	for(size_t i =0; i< cycle_size; i++) {
		double x = radius * cos(i*step);
		if (x <  std::numeric_limits<double>::epsilon() && 
				    x > -std::numeric_limits<double>::epsilon()) {
			  x = 0.0;
		}
		double y = radius * sin(i*step);
		if (y <  std::numeric_limits<double>::epsilon() && 
				    y > -std::numeric_limits<double>::epsilon()) {
			  y = 0.0;
		}
		coord_t coord = {x,y};
		cycle_coordinates.push_back(coord);
	}

	//std::cout << "Step: " << step << std::endl;
	//std::cout << "Step FULL: " << step * cycle.size() << std::endl;
	//for(auto c : cycle_coordinates){
	//	std::cout << c << ' ';
	//}
	//std::cout << std::endl;
	
	return cycle_coordinates;
}

/**
 *
 */
template <typename Graph>
auto tutteDraw(IndexedGraph<Graph> g) -> DrawnGraph<Graph>{
	auto cycle = findCycle(g).value();
	//for(auto&& v : cycle)
	//	std::cout << v << ' ';
	//std::cout << std::endl;
	auto cycle_coordinates = drawCycle(cycle.size());
	auto coordinates = tutteDrawImpl(g,cycle,cycle_coordinates);
	return DrawnGraph(std::move(g),std::move(coordinates));
}

/**
 * A wrapper around tutteDrawImpl.
 * It chooses the largest facial cycle for the algorithm.
 */
template <typename Graph>
auto tuttePlanarDraw(PlanarGraph<Graph> g) -> DrawnGraph<Graph>{
	auto facial_cycle = findLargestFacialCycle(g);
	auto cycle_coordinates = drawCycle(facial_cycle.size());
	auto coordinates = tutteDrawImpl(g,facial_cycle,cycle_coordinates);
	return DrawnGraph(std::move(g),std::move(coordinates));
}

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
auto tutteDrawBoostImpl(PlanarGraph<Graph> g, const Range& facial_cycle) -> DrawnGraph<Graph>{

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
auto tutteDrawBoost(PlanarGraph<Graph> g) -> DrawnGraph<Graph>{
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
auto drawFlattenedGraph(PlanarGraph<Graph> g, size_t original_vcount, std::function<DrawnGraph<Graph>(PlanarGraph<Graph>)> draw_method = tuttePlanarDraw<Graph>) -> DrawnGraph<Graph>{

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

auto intersect(coord_t a,coord_t b, coord_t c, coord_t d) -> bool{
	double det = (b.x - a.x) * (c.y - d.y) - ((b.y - a.y)*(c.x - d.x));

	double det1 = (c.x - a.x) * (c.y - d.y) - ((c.y - a.y)*(c.x - d.x));
	double det2 = (b.x - a.x) * (c.y - a.y) - ((b.y - a.y)*(c.x - a.x));

	double s = det1/det;
	double t = det2/det;

	return (0 + std::numeric_limits<double>::epsilon() <= s && s <= 1-std::numeric_limits<double>::epsilon()) &&
	 (0 + std::numeric_limits<double>::epsilon() <= t && t <= 1-std::numeric_limits<double>::epsilon());
}

//TODO could use the nlog n multiple line intersection algorithm here...
template <typename Graph>
auto isStraightLineDrawing(const DrawnGraph<Graph>& g) -> bool{
	auto cross = [&g](auto e,auto f){
		auto [u_e,v_e] = g.endpoints(e);
		auto [u_f,v_f] = g.endpoints(f);
		return intersect(g.coordinates[g.index(u_e)],
				g.coordinates[g.index(v_e)],
				g.coordinates[g.index(u_f)],
				g.coordinates[g.index(v_f)]);
	};
	for(auto&& c : iter::combinations(g.edges(),2)){
		auto e = *(c.begin());
		auto f = *(c.begin()+1);
		//std::cout << e << ' ' << f << ' ' << std::boolalpha << cross(e,f) << std::endl;
		if(cross(e,f))
			return false;
	}

	return true;
}


} //namespace

