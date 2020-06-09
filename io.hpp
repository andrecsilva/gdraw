#include <string>

#include <boost/graph/graphviz.hpp>

#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>

template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

template <typename Graph>	
using rotations_t = typename std::vector< std::vector< edge_t<Graph> > >;

struct coord_t{
	float x;
	float y;

	coord_t() {x=0;y=0;}

	coord_t(float _x, float _y) : x(_x), y(_y) {}


	friend std::ostream& operator<<(std::ostream& out, const coord_t& c);
};

std::ostream& operator<<(std::ostream& out, const coord_t& c){
	out << c.x << "," << c.y;
	return out;
}

struct cubicSpline{
	coord_t from;
	coord_t control1;
	coord_t control2;
	coord_t to;

	cubicSpline(){}

	cubicSpline(coord_t _from, coord_t _control1, coord_t _control2, coord_t _to) : from(_from), control1(_control1), control2(_control2), to(_to) {}

	friend std::ostream& operator<<(std::ostream& out, const cubicSpline& spline);
};

std::ostream& operator<<(std::ostream& out, const cubicSpline& spline){
	out << spline.from << " " << spline.control1 << " " << spline.control2 << " " << spline.to;
	return out;
}



/***
 * Prints the vertices and edges of a graph.
 */
template <typename Graph>
void printGraph(const Graph& g) noexcept{
	
	typename boost::graph_traits<Graph>::vertex_iterator vi;

	std::cout << "Vertices:" << std::endl;

	//auto edgei_map = get( boost::edge_index, g);
	//auto vertexi_map = get( boost::vertex_index, g);

	for(vi = vertices(g).first; vi!=vertices(g).second; ++vi){
		std::cout << *vi << " ";
	}

	typename boost::graph_traits<Graph>::edge_iterator ei;

	std::cout << std::endl << "Edges:" << std::endl;

	auto edgei_map = get( boost::edge_index, g);

	for(ei = edges(g).first; ei!=edges(g).second; ++ei){
		std::cout << "[" << boost::get(edgei_map,*ei) << "]" <<*ei << " "; 
	}

	std::cout << std::endl;
}

/*
 * Assumes g is planar. Returns a vector containing the coordinates of each vertex v.
 */
template <typename Graph>
std::vector<coord_t> draw(Graph& g) noexcept{
	//copy graph
	Graph gprime= Graph{g};

	auto edgei_map = get( boost::edge_index, gprime);
	typename boost::graph_traits<Graph>::edges_size_type ecount = num_edges(gprime);
	typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;

	//Find the original edge with index 0.
	//We want the newly added edges with index >= num_edges(g)
	edge_t<Graph> original_0_edge;
	for(boost::tie(ei,ei_end) = edges(gprime);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0)
			original_0_edge = *ei;
			
	rotations_t<Graph> embedding(num_vertices(gprime));
	//typedef std::vector< typename boost::graph_traits<Graph>::edge_descriptor > vec_t;
	//std::vector<vec_t> embedding(num_vertices(g));
	

	//make biconnected
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = gprime
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	//std::cout << "Rotations:" << std::endl;

	//for(auto i = embedding.begin(); i!=embedding.end();i++){
	//	std::cout << std::distance(embedding.begin(),i) << std::endl;
	//	for(auto ei = i->begin(); ei!=i->end();ei++)	
	//		std::cout << *ei << " ";
	//std::cout << std::endl;
	//}

	make_biconnected_planar(gprime,&embedding[0]);

	//std::cout << "Make Biconnected" << std::endl;
	//printGraph(gprime);

	//Add edge_index for the newly added edges
	for(boost::tie(ei,ei_end) = edges(gprime);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0 && *ei!=original_0_edge)
			put(edgei_map,*ei,ecount++);


	//make maximal
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = gprime
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	//TODO see what happens if the embedding here is reused... boost example passes &embedding[0]...
	make_maximal_planar(gprime,&embedding[0]);

	//std::cout << "Make Maximal" << std::endl;
	//printGraph(gprime);
	//Add edge_index for the newly added edges
	for(boost::tie(ei,ei_end) = edges(gprime);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0 && *ei!=original_0_edge)
			put(edgei_map,*ei,ecount++);

	//printGraph(gprime);
	//canonical ordering	
	boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = gprime
		,boost::boyer_myrvold_params::embedding = &embedding[0]
		);

	std::vector<vertex_t<Graph> > ordering;
	planar_canonical_ordering(gprime, &embedding[0], std::back_inserter(ordering));

	
	//get drawing
	
	std::vector<coord_t> coordinates(num_vertices(gprime));

	chrobak_payne_straight_line_drawing(gprime,
			  embedding, 
			  ordering.begin(),
			  ordering.end(),
			  &coordinates[0]);

	//printGraph(gprime);

	return coordinates;
}

/*
 * Reads a DOT file from stdin and returns the graph.
 */

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

template <class Pos,class Color>
class PosColorEdgeWriter{
	public:
		PosColorEdgeWriter(Pos _pos,Color _color) : pos(_pos), color(_color) {}
		template <class Edge>
			void operator()(std::ostream& out, const Edge& e){ 

				if((pos.count(e) > 0) || (color.count(e) > 0)){
					out << "[";
					if(pos.count(e) > 0)
						out << "pos=\"" << pos.at(e) << "\"";
					if(color.count(e) > 0){
						if(pos.count(e) >0)
							out << ",";
						out << "color=" << color.at(e);
					}
					out << "]";
				}
			}
	private:
		Pos pos;
		Color color;

};

template <class Pos,class Color>
class PosColorVertexWriter{
	public:
		PosColorVertexWriter(Pos _pos,Color _color) : pos(_pos), color(_color) {}
		template <class Vertex>
			void operator()(std::ostream& out, const Vertex& v){ 

					out << "[";
					out << "pos=\"" << pos.at(v) << "\"";
					if(!color.empty() && color.at(v) != ""){
						out << ",";
						out << "color=" << color.at(v);
					}
					out << "]";
			}
	private:
		Pos pos;
		Color color;

};
/*
 * Given a pair of endpoints and a crossing point it, returns a cubic spline with control points with distance epsilon from the crossing point.
 */
cubicSpline
crossingSpline(const coord_t& xpoint, const std::pair<coord_t,coord_t>& e, const float epsilon=0.2) noexcept {
	const coord_t p1 {xpoint.x + (e.first.x - xpoint.x ) * epsilon, xpoint.y +  (e.first.y - xpoint.y) * epsilon};
	//coord_t p2 {xpoint.x + (e.first.x - xpoint.x ) * epsilon/2, xpoint.y +  (e.first.y - xpoint.y) * epsilon/2};
	const coord_t p2 {xpoint.x + (e.second.x - xpoint.x ) * epsilon, xpoint.y +  (e.second.y - xpoint.y) * epsilon};
	cubicSpline spline { e.first, p1, p2, e.second};
	return spline;
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
void writeDOT(std::ostream& out,
	       	const Graph& g,
	       	const std::vector<coord_t>& coordinates,
		const std::vector<std::string>& vertex_colors = {},
		const std::map<edge_t<Graph>,cubicSpline>& xcoordinates = {},
		const std::map<edge_t<Graph>,std::string>& edge_colors = {}) noexcept{

	PosColorVertexWriter<std::vector<coord_t>,std::vector<std::string>> vpw{coordinates,vertex_colors};
	PosColorEdgeWriter<std::map<edge_t<Graph>,cubicSpline>,std::map<edge_t<Graph>,std::string>> epw{xcoordinates,edge_colors};
	boost::write_graphviz(out,g,vpw,epw);

}

