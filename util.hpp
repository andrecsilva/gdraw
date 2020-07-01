#include <iostream>
#include <functional>
#include <algorithm>

#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "graph_types.hpp"

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


template <typename Graph>
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

template <typename Graph>
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

template <typename Graph> 
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

bool enumerate(std::vector<bool>& mask,
		std::function<bool(std::vector<bool>&)> execute,
		size_t& min_size,
	       	size_t& max_size,
		size_t& size,
		size_t& index,
		bool& found)
{

	if(!found){
		if(index<mask.size()){
			if(size < max_size){
				mask[index] = true;
				size++;
				index++;
				enumerate(mask,execute,min_size,max_size,size,index,found);
				mask[index-1] = false;
				size--;
				enumerate(mask,execute,min_size,max_size,size,index,found);
				index--;
			}else
				found = execute(mask);
		}
		else
			if(size>=min_size)
				found = execute(mask);

	}
	return found;
}

/*
 * Executes execute for each subset of cont with size between min_size and max_size.
 * The function execute should take the Container cont and a mask that represents the sub
 * set of cont.
 */
template <typename Container>
bool boundedSubsetsExecute(Container& cont,
		std::function<bool(Container&,std::vector<bool>&)> execute,
		size_t min_size,
		size_t max_size=0){
	auto bound_execute = std::bind(execute,cont,std::placeholders::_1);
	std::vector<bool> mask (cont.size(),false);
	size_t size = 0;
	size_t index = 0;
	bool found = false;
	return enumerate(mask,bound_execute,min_size,max_size,index,size,found);
}

/*
 * Take a set of edges of g and returns the edges not in the set.
 */
template <typename Graph, typename Container>
std::vector<edge_t<Graph>> coSubgraphEdges(const Graph& g, Container& subgraph_edges){
	std::vector<edge_t<Graph>> cs_edges;

	auto edgei_map = get(boost::edge_index,g);

	std::vector<bool> in_subgraph (num_edges(g),false);
	auto in_subgraph_map = make_iterator_property_map(in_subgraph.begin(), edgei_map);

	for(auto& e : subgraph_edges){
		put(in_subgraph_map,e,true);
	}

	for(auto [ei,ei_end] = edges(g);ei!=ei_end;ei++){
		edge_t<Graph> e = *ei;
		if(!get(in_subgraph_map,e))
			cs_edges.push_back(e);
	}
	return cs_edges;
}

/*
 * Returns a random spanning tree of g.
 * It is just a wrap around Boost's function.
 */
template <typename Graph>
std::vector<edge_t<Graph>> randomSpanningTree(const Graph& g){	
	std::vector<vertex_t<Graph>> parent(num_vertices(g),boost::graph_traits<Graph>::null_vertex());
	std::vector<edge_t<Graph>> tree_edges;

	boost::mt19937 gen(time(0));
	boost::random_spanning_tree(g,gen,boost::predecessor_map(&parent[0]));

	for (auto [vi,vi_end] = vertices(g); vi!=vi_end; vi++){
		if(parent[*vi] != boost::graph_traits<Graph>::null_vertex()){
			tree_edges.push_back(edge(*vi,parent[*vi],g).first);
		}
	}

	return tree_edges;
}


