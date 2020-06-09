#include <iostream>

#include <boost/graph/boyer_myrvold_planar_test.hpp>


template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

template <typename Graph>	
using rotations_t = typename std::vector< std::vector< edge_t<Graph> > >;



/**
 * Removes isolated vertices (i.e. degree 0) from the graph.
 */
template <typename Graph>
void removeIsolatedVertices(Graph& g){
	//Reverse order, as the indexes are rearranged in the graph to make it contiguous
	for(int i = num_vertices(g)-1; i>0 ;i--){
		auto vi = vertex(i,g);
		if(out_degree(vi,g)==0)
			remove_vertex(vi,g);
	}
}

/**
 * A simple wrapper for the boyer_myrvold_planarity_test in boost. 
 * It assumes the graph has the edge_index and vertex_index as internal properties.
 */
template <typename Graph>
bool isPlanar(const Graph& g
		,std::vector < edge_t<Graph> >&  kuratowski_edges
		,rotations_t<Graph>& rotations
	     ) noexcept{
	
	//TODO change this to make_iterator...
	//From the boost example, somehow gets an iterator of rotations out of this
	using rotations_pmap_t = typename boost::iterator_property_map <
				typename rotations_t<Graph>::iterator
				,typename boost::property_map <Graph,boost::vertex_index_t>::type
				>;
	
	rotations_pmap_t rotations_pmap(rotations.begin(),get(boost::vertex_index,g));

	return boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		,boost::boyer_myrvold_params::embedding = rotations_pmap
		,boost::boyer_myrvold_params::kuratowski_subgraph = std::back_inserter(kuratowski_edges)
		);
}

template <typename Graph>
bool isPlanar(const Graph& g
		,rotations_t<Graph>& rotations
	     ) noexcept{
	
	//Concept type class using rotations_t
	using rotations_pmap_t = typename boost::iterator_property_map <
				typename rotations_t<Graph>::iterator
				,typename boost::property_map <Graph,boost::vertex_index_t>::type
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
template <typename Graph>
bool isPlanar(const Graph& g) noexcept{

	return boyer_myrvold_planarity_test(
		boost::boyer_myrvold_params::graph = g
		);
}

template <typename Graph>
edge_t<Graph> addEdgeWithIndex(Graph& g
		,const vertex_t<Graph>& target
		,const vertex_t<Graph>& source
		,const int& index
		,std::vector<edge_t<Graph>>& edges_by_id
	       	,typename boost::property_map<Graph, boost::edge_index_t>::type& edgei_map
		){
	edge_t<Graph> e = add_edge(target,source,g).first;
	put(edgei_map,e,index);
	edges_by_id.at(index) = e;
	return e;
}

template <typename Graph>
void fakeCross(Graph& g
		,edge_t<Graph>& ei
		,const int ei_index
		,edge_t<Graph>& ej
		,const int ej_index
		,std::vector<edge_t<Graph>>& edges_by_id
	       	,typename boost::property_map<Graph, boost::edge_index_t>::type& edgei_map
		,typename boost::graph_traits<Graph>::vertices_size_type& vcount
		,typename boost::graph_traits<Graph>::edges_size_type& ecount
		){

	//caution: this invalidates descriptors..
	remove_edge(ei,g);
	remove_edge(ej,g);

	//reuses original indexes
	vertex_t<Graph> v = vertex(vcount,g);
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
template <typename Graph>
bool leqXnumber1(Graph& g
	       	,rotations_t<Graph>& _out_rotations
		,std::vector<edge_t<Graph>>& edges_by_id
	       	,typename boost::property_map<Graph, boost::edge_index_t>::type& edgei_map
		,typename boost::graph_traits<Graph>::vertices_size_type& vcount
		,typename boost::graph_traits<Graph>::edges_size_type& ecount
		) noexcept{

	std::vector<edge_t<Graph>> kuratowski_edges;

	rotations_t<Graph> embedding {num_vertices(g)};

	//TODO a graph with only one crossing has at most 3n-5 edges
	if(isPlanar(g,kuratowski_edges, embedding)){
		_out_rotations = std::move(embedding);
		return true;
	}

	//embedding.resize(num_vertices(g)+1);
	//Essentially the same as the general case, but we use the edges of the kuratowski subgraph instead
	for(std::size_t i =0; i < kuratowski_edges.size(); i++){
		edge_t<Graph> ei = edges_by_id.at(i);
		for(std::size_t j = i+1; j < kuratowski_edges.size() ;  j++){
			edge_t<Graph> ej = edges_by_id.at(j);
			//std::cout << ei << " x " << ej << std::endl;
			if (target(ei,g) != source(ej,g)
					&& target(ei,g) != target(ej,g)
					&& source(ei,g) != source(ej,g)
					&& source(ei,g) != target(ej,g)
			   ){

				vertex_t<Graph> eit = target(ei,g);
				vertex_t<Graph> eis = source(ei,g);

				vertex_t<Graph> ejt = target(ej,g);
				vertex_t<Graph> ejs = source(ej,g);

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


template <typename Graph>
bool leqXnumberkRecursion(Graph& g
	       	,rotations_t<Graph>& _out_rotations
		,std::vector<edge_t<Graph>>& edges_by_id
	       	,typename boost::property_map<Graph, boost::edge_index_t>::type& edgei_map
		,typename boost::graph_traits<Graph>::vertices_size_type& vcount
		,typename boost::graph_traits<Graph>::edges_size_type& ecount
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
			edge_t<Graph> ei = edges_by_id.at(i);
			for(std::size_t j = i+1; j < ecount ;  j++){
				edge_t<Graph> ej = edges_by_id.at(j);
				//std::cout << ei << " x " << ej << std::endl;
				if (target(ei,g) != source(ej,g)
						&& target(ei,g) != target(ej,g)
						&& source(ei,g) != source(ej,g)
						&& source(ei,g) != target(ej,g)
				   ){

					vertex_t<Graph> eit = target(ei,g);
					vertex_t<Graph> eis = source(ei,g);

					vertex_t<Graph> ejt = target(ej,g);
					vertex_t<Graph> ejs = source(ej,g);

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


template <typename Graph>
bool leqXnumberk(Graph& g, rotations_t<Graph>& _out_rotations,  int k) noexcept{

	//Build a vector indexing the edges by their id.
	//Used to avoid iterator/edge description invalidation caused by the removal of edges
	typename boost::property_map<Graph, boost::edge_index_t>::type edgei_map = get(boost::edge_index, g) ;
	
	//size = num_edges + all the possible extra edges (k*2)
	std::vector<edge_t<Graph>> edges_by_id {num_edges(g)+k*2};

	typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
	for(boost::tie(ei,ei_end) = edges(g);ei!=ei_end;ei++){
		auto i = get(edgei_map,*ei);
		edges_by_id.at(i) = *ei;
	}
	//add all the possible extra vertices at once, to avoid repeatedly doing so in the recursion
	typename boost::graph_traits<Graph>::vertices_size_type vcount = num_vertices(g);
	typename boost::graph_traits<Graph>::edges_size_type ecount = num_edges(g);

	for (auto i=0; i<k;i++)
		add_vertex(g);

	bool answer = leqXnumberkRecursion(g,_out_rotations,edges_by_id,edgei_map,vcount,ecount,k);

	//removes extra added vertices and resizes the rotations if necessary
	if(answer){
		removeIsolatedVertices(g);
		_out_rotations.resize(num_vertices(g));
	}

	return answer;
}



