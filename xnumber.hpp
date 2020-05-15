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

template <typename G>
edge_t<G> addEdgeWithIndex(G& g
		,const vertex_t<G>& target
		,const vertex_t<G>& source
		,const int& index
		,std::vector<edge_t<G>>& edges_by_id
	       	,typename boost::property_map<G, boost::edge_index_t>::type& edgei_map
		){
	edge_t<G> e = add_edge(target,source,g).first;
	put(edgei_map,e,index);
	edges_by_id.at(index) = e;
	return e;
}

template <typename G>
void fakeCross(G& g
		,edge_t<G>& ei
		,const int ei_index
		,edge_t<G>& ej
		,const int ej_index
		,std::vector<edge_t<G>>& edges_by_id
	       	,typename boost::property_map<G, boost::edge_index_t>::type& edgei_map
		,typename boost::graph_traits<G>::vertices_size_type& vcount
		,typename boost::graph_traits<G>::edges_size_type& ecount
		){

	//caution: this invalidates descriptors..
	remove_edge(ei,g);
	remove_edge(ej,g);

	//reuses original indexes
	vertex_t<G> v = vertex(vcount,g);
	addEdgeWithIndex(g,source(ei,g),v,ei_index,edges_by_id,edgei_map);
	addEdgeWithIndex(g,source(ej,g),v,ej_index,edges_by_id,edgei_map);

	addEdgeWithIndex(g,target(ei,g),v,ecount++,edges_by_id,edgei_map);
	addEdgeWithIndex(g,target(ej,g),v,ecount++,edges_by_id,edgei_map);
	vcount++;
}


/*
 * Checks if the graph has crossing number 1.
 * If it does, rotations contains the order type of the graph and the crossing vertices.
 * Modifies the graph.
 */
template <typename G>
bool leqXnumber1(G& g
	       	,rotations_t<G>& _out_rotations
		,std::vector<edge_t<G>>& edges_by_id
	       	,typename boost::property_map<G, boost::edge_index_t>::type& edgei_map
		,typename boost::graph_traits<G>::vertices_size_type& vcount
		,typename boost::graph_traits<G>::edges_size_type& ecount
		) noexcept{

	std::vector<edge_t<G>> kuratowski_edges;

	rotations_t<G> embedding {num_vertices(g)};

	//TODO a graph with only one crossing has at most 3n-5 edges
	if(isPlanar(g,kuratowski_edges, embedding)){
		_out_rotations = std::move(embedding);
		return true;
	}

	//embedding.resize(num_vertices(g)+1);
	//Essentially the same as the general case, but we use the edges of the kuratowski subgraph instead
	for(std::size_t i =0; i < kuratowski_edges.size(); i++){
		edge_t<G> ei = edges_by_id.at(i);
		for(std::size_t j = i+1; j < kuratowski_edges.size() ;  j++){
			edge_t<G> ej = edges_by_id.at(j);
			//std::cout << ei << " x " << ej << std::endl;
			if (target(ei,g) != source(ej,g)
					&& target(ei,g) != target(ej,g)
					&& source(ei,g) != source(ej,g)
					&& source(ei,g) != target(ej,g)
			   ){

				vertex_t<G> eit = target(ei,g);
				vertex_t<G> eis = source(ei,g);

				vertex_t<G> ejt = target(ej,g);
				vertex_t<G> ejs = source(ej,g);

				fakeCross(g,ei,i,ej,j,edges_by_id,edgei_map,vcount,ecount);

				if (isPlanar(g,embedding)){
					_out_rotations = std::move(embedding);
					return true;
				}

				//Uncross
				//TODO O(E/V) here
				//TODO This could be constant time by pop()ing the vertices
				vcount--;
				clear_vertex(vertex(vcount,g),g);
				ecount-=2;

				//revalidate edge descriptors
				ei = addEdgeWithIndex(g,eis,eit,i,edges_by_id,edgei_map);
				ej = addEdgeWithIndex(g,ejs,ejt,j,edges_by_id,edgei_map);
			}
			//else std::cout << "Not disjoint" << std::endl;
		}
	}
	return false;

}


template <typename G>
bool leqXnumberkRecursion(G& g
	       	,rotations_t<G>& _out_rotations
		,std::vector<edge_t<G>>& edges_by_id
	       	,typename boost::property_map<G, boost::edge_index_t>::type& edgei_map
		,typename boost::graph_traits<G>::vertices_size_type& vcount
		,typename boost::graph_traits<G>::edges_size_type& ecount
		,int k
		) noexcept{
	//std::cout << "Depth: " << k << std::endl;

	if(k<=1){
		return leqXnumber1(g,_out_rotations,edges_by_id,edgei_map,vcount,ecount);
	}
	if(leqXnumberkRecursion(g,_out_rotations,edges_by_id,edgei_map,vcount,ecount,k-1))
		return true;
	else{
		//std::cout << "Depth: " << k << std::endl;
		auto ecount  = num_edges(g);
		for(std::size_t i =0; i < ecount; i++){
			edge_t<G> ei = edges_by_id.at(i);
			for(std::size_t j = i+1; j < ecount ;  j++){
				edge_t<G> ej = edges_by_id.at(j);
				//std::cout << ei << " x " << ej << std::endl;
				if (target(ei,g) != source(ej,g)
						&& target(ei,g) != target(ej,g)
						&& source(ei,g) != source(ej,g)
						&& source(ei,g) != target(ej,g)
				   ){

					vertex_t<G> eit = target(ei,g);
					vertex_t<G> eis = source(ei,g);

					vertex_t<G> ejt = target(ej,g);
					vertex_t<G> ejs = source(ej,g);

					fakeCross(g,ei,i,ej,j,edges_by_id,edgei_map,vcount,ecount);

					if (leqXnumberkRecursion(g,_out_rotations,edges_by_id,edgei_map,vcount,ecount,k-1)){
						return true;
					}

					
					//Uncross
					//TODO O(E/V) here
					//TODO This could be constant time by pop()ing the vertices
					vcount--;
					clear_vertex(vertex(vcount,g),g);
					ecount-=2;

					//revalidate edge descriptors
					ei = addEdgeWithIndex(g,eis,eit,i,edges_by_id,edgei_map);
					ej = addEdgeWithIndex(g,ejs,ejt,j,edges_by_id,edgei_map);
				}
				//else std::cout << "Not disjoint" << std::endl;
			}
		}

	}
	return false;
}


template <typename G>
bool leqXnumberk(G& g, rotations_t<G>& _out_rotations,  int k) noexcept{

	//Build a vector indexing the edges by their id.
	//Used to avoid iterator/edge description invalidation caused by the removal of edges
	typename boost::property_map<G, boost::edge_index_t>::type edgei_map = get(boost::edge_index, g) ;
	
	//size = num_edges + all the possible extra edges (k*2)
	std::vector<edge_t<G>> edges_by_id {num_edges(g)+k*2};

	typename boost::graph_traits<G>::edge_iterator ei, ei_end;
	for(boost::tie(ei,ei_end) = edges(g);ei!=ei_end;ei++){
		auto i = get(edgei_map,*ei);
		edges_by_id.at(i) = *ei;
	}
	//add all the possible extra vertices at once, to avoid repeatedly doing so in the recursion
	typename boost::graph_traits<G>::vertices_size_type vcount = num_vertices(g);
	typename boost::graph_traits<G>::edges_size_type ecount = num_edges(g);

	for (auto i=0; i<k;i++)
		add_vertex(g);

	
	return leqXnumberkRecursion(g,_out_rotations,edges_by_id,edgei_map,vcount,ecount,k);
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
