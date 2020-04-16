#include "xnumber.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

using Graph = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,int>
	,boost::property<boost::edge_index_t,int>
	>; 

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

Graph getV8() noexcept{
	Graph g(8);
	add_edge(0, 1, g);
	add_edge(1, 2, g);
	add_edge(2, 3, g);
	add_edge(3, 4, g);
	add_edge(4, 5, g);
	add_edge(5, 6, g);
	add_edge(6, 7, g);
	add_edge(7, 0, g);

	add_edge(0, 4, g);
	add_edge(1, 5, g);
	add_edge(2, 6, g);
	add_edge(3, 7, g);

	auto edgei_map = get( boost::edge_index, g);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;
	typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
	for(boost::tie(ei,ei_end) = edges(g);ei!=ei_end;ei++)
		put(edgei_map,*ei,ecount++);
	return g;
}

Graph getKn(int n) noexcept{

	Graph Kn(n);

	for (int i=0; i<n;i++)
		for(int j=i+1;j<n;j++)
			add_edge(i,j,Kn);

	auto edgei_map = get( boost::edge_index, Kn);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(ei = edges(Kn).first; ei!=edges(Kn).second; ++ei)
		put(edgei_map,*ei,ecount++);

	return Kn;
}

Graph getKpq(int p,int q) noexcept
{

	Graph Kpq(p+q);

	for (int i=0; i<p+q;i+=2)
		for(int j=1;j<p+q;j+=2)
			add_edge(i,j,Kpq);

	auto edgei_map = get( boost::edge_index, Kpq);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(ei = edges(Kpq).first; ei!=edges(Kpq).second; ++ei)
		put(edgei_map,*ei,ecount++);

	return Kpq;
}

/*
 * Reads a DOT file from stdin and returns the graph.
 */

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

//TODO I don't like this exception...
template <class Pos>
class PosWriter{
	public:
		PosWriter(Pos _pos) : pos(_pos) {}
		template <class VertexOrEdge>
			void operator()(std::ostream& out, const VertexOrEdge& v){ 
				try{
					pos.at(v);
					out << "[pos=\"" << pos.at(v) << "\"]";
				}
				catch(const std::out_of_range& oor){

				}
			}
	private:
		Pos pos;

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

template <typename G>
void writeDOT(std::ostream& out, G& g, const std::vector<coord_t>& coordinates) noexcept{
	PosWriter<std::vector<coord_t>> vpw{coordinates};
	boost::write_graphviz(out,g,vpw);
}


template <typename G>
void writeDOT(std::ostream& out, const G& g, const G& gp, const rotations_t<G>& rotations, const std::vector<coord_t>& coordinates ) noexcept{
	auto ognvertices = num_vertices(g);

	//TODO change to a unordered_map using a simple hash function on a pair of integers
	std::map<edge_t<G>,cubicSpline> xcoordinates;

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
	

	PosWriter<std::vector<coord_t>> vpw{coordinates};
	PosWriter<std::map<edge_t<G>,cubicSpline>> epw{xcoordinates};
	boost::write_graphviz(out,g,vpw,epw);
}
