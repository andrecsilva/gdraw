#include <iostream>

#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>


template <typename G>	
using edge_t = typename boost::graph_traits<G>::edge_descriptor;

template <typename G>	
using vertex_t = typename boost::graph_traits<G>::vertex_descriptor;

template <typename G>	
using rotations_t = typename std::vector< std::vector< edge_t<G> > >;

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


/***
 * Prints the vertices and edges of a graph.
 */
template <typename G>
void printGraph(const G& g) noexcept{
	
	typename boost::graph_traits<G>::vertex_iterator vi;

	std::cout << "Vertices:" << std::endl;

	//auto edgei_map = get( boost::edge_index, g);
	//auto vertexi_map = get( boost::vertex_index, g);

	for(vi = vertices(g).first; vi!=vertices(g).second; ++vi){
		std::cout << *vi << " ";
	}

	typename boost::graph_traits<G>::edge_iterator ei;

	std::cout << std::endl << "Edges:" << std::endl;

	auto edgei_map = get( boost::edge_index, g);

	for(ei = edges(g).first; ei!=edges(g).second; ++ei){
		std::cout << "[" << boost::get(edgei_map,*ei) << "]" <<*ei << " "; 
	}

	std::cout << std::endl;
}

/**
 * A simple wrapper for the boyer_myrvold_planarity_test in boost. 
 * It assumes the graph has the edge_index and vertex_index as internal properties.
 */
template <typename G>
bool isPlanar(const G& g
		,std::vector < edge_t<G> >&  kuratowski_edges
		,rotations_t<G>& rotations
	     ) noexcept{
	
	//From the boost example, somehow gets an iterator of rotations out of this
	using rotations_pmap_t = typename boost::iterator_property_map <
				typename rotations_t<G>::iterator
				,typename boost::property_map <G,boost::vertex_index_t>::type
				>;
	
	rotations_pmap_t rotations_pmap(rotations.begin(),get(boost::vertex_index,g));

	return boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		,boost::boyer_myrvold_params::embedding = rotations_pmap
		,boost::boyer_myrvold_params::kuratowski_subgraph = std::back_inserter(kuratowski_edges)
		);
}

template <typename G>
bool isPlanar(const G& g
		,rotations_t<G>& rotations
	     ) noexcept{
	
	//Concept type class using rotations_t
	using rotations_pmap_t = typename boost::iterator_property_map <
				typename rotations_t<G>::iterator
				,typename boost::property_map <G,boost::vertex_index_t>::type
				>;
	
	rotations_pmap_t rotations_pmap(rotations.begin(),get(boost::vertex_index,g));

	return boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		,boost::boyer_myrvold_params::embedding = rotations_pmap
		);
}

/**
 * A simple wrapper for the boyer_myrvold_planarity_test in boost. 
 * It assumes the graph has the edge_index and vertex_index as internal properties.
 */
template <typename G>
bool isPlanar(const G& g) noexcept{

	return boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		);
}

/*
 * Checks if the graph has crossing number 1.
 * If it does, rotations contains the order type of the graph and the crossing vertices.
 * Modifies the graph.
 */
template <typename G>
bool leqXnumber1(G& g
		,rotations_t<G>& _out_rotations
		) noexcept{


	std::vector<edge_t <G>> kuratowski_edges;

	auto k = num_vertices(g);

	rotations_t<G> embedding {k};

	//TODO a graph with only one crossing has at most 3n-5 edges
	if(isPlanar(g,kuratowski_edges, embedding)){
		_out_rotations = std::move(embedding);
		return true;
	}

	embedding.resize(k+1);

	//adds crossing vertex
	add_vertex(g);


	auto edgei_map = get(boost::edge_index, g);
	auto ecount  = num_edges(g);
	
	for(typename std::vector<edge_t<G>>::size_type i = 0; i < kuratowski_edges.size(); ++i){
		auto ei = kuratowski_edges.at(i);
		for(typename std::vector<edge_t<G> >::size_type j = 0; j < kuratowski_edges.size(); ++j){
			auto ej = kuratowski_edges.at(j);
			// Don't try to cross edges with a common endpoint
			if (target(ei,g) != source(ej,g)
				        && target(ei,g) != target(ej,g)
				       	&& source(ei,g) != source(ej,g)
				       	&& source(ei,g) != target(ej,g)
					){
				//std::cout << ei << " x " << ej << std::endl;
				auto eit = target(ei,g);
				auto eis = source(ei,g);

				auto ejt = target(ej,g);
				auto ejs = source(ej,g);

				//caution: this invalidates descriptors..
				int eiindex = get(edgei_map,ei);
				remove_edge(ei,g);
				int ejindex = get(edgei_map,ej);
				remove_edge(ej,g);

				//reuses original indexes
				auto tik = add_edge(eit,k,g).first;
				put(edgei_map,tik,eiindex);
				auto tjk = add_edge(ejt,k,g).first;
				put(edgei_map,tjk,ejindex);

				auto sik = add_edge(eis,k,g).first;
				put(edgei_map,sik,ecount++);
				auto sjk = add_edge(ejs,k,g).first;
				put(edgei_map,sjk,ecount++);

				//TODO caution about edge indexes here...
				//TODO possible solution: unordered map maintaining edge_indexes
				//TODO add_edge(e,g) -> maintain original edge index if possible
				
				if (isPlanar(g,embedding)){
					_out_rotations = std::move(embedding);
					return true;
				}

				//TODO O(E/V) here
				//TODO check if there's a more efficient way to do this, maybe copy the entire graph and modify the copy?
				//TODO This could be constant time by pop()ing the out_vectors of the known vertices
				
				remove_edge(sik,g);
				remove_edge(tik,g);
				remove_edge(sjk,g);
				remove_edge(tjk,g);
				ecount-=2;

				ei = add_edge(eis,eit,g).first;
				ej = add_edge(ejs,ejt,g).first;

				put(edgei_map,ei,eiindex);
				put(edgei_map,ej,ejindex);

				//revalidate descriptors at the vector
				//TODO will this leak memory? probably not.
				kuratowski_edges.at(i) = ei;
				kuratowski_edges.at(j) = ej;
				//printGraph(g);
			}
			//else std::cout << ei << " NOT " << ej << std::endl;
			

		}
	}
	return false;
}

template <typename G>
void isXnumberk(G& g, int k)noexcept{
}

/*
 * Assumes g is planar
 */
template <typename G>
std::vector<coord_t> draw(G& g) noexcept{
	//copy graph
	G gprime= G(g);


	auto edgei_map = get( boost::edge_index, gprime);
	typename boost::graph_traits<G>::edges_size_type ecount = num_edges(gprime);
	typename boost::graph_traits<G>::edge_iterator ei, ei_end;

	//Find the original edge with index 0.
	//We want the newly added edges with index >= num_edges(g)
	edge_t<G> original_0_edge;
	for(boost::tie(ei,ei_end) = edges(gprime);ei!=ei_end;ei++)
		if(get(edgei_map,*ei)==0)
			original_0_edge = *ei;
			
	rotations_t<G> embedding(num_vertices(gprime));
	//typedef std::vector< typename boost::graph_traits<G>::edge_descriptor > vec_t;
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

	std::vector<vertex_t<G> > ordering;
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

/**
 * Removes isolated vertices (i.e. degree 0) from the graph.
 */
template <typename G>
void removeIsolatedVertices(G& g){
	//Reverse order, as the indexes are rearranged in the graph to make it contiguous
	for(int i = num_vertices(g)-1; i>0 ;i--){
		auto vi = vertex(i,g);
		if(out_degree(vi,g)==0)
			remove_vertex(vi,g);
	}
}
