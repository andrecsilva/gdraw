/**
 * Functions for generating common graph families.
 */
#pragma once

namespace gdraw{

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

	for(auto [ei,ei_end] = edges(g); ei!=ei_end;ei++)
		put(edgei_map,*ei,ecount++);
	return g;
}

/**
 * Generates a complete graph on n vertices.
 */
template <typename Graph>
Graph getKn(int n) noexcept{

	Graph Kn(n);

	for (int i=0; i<n;i++)
		for(int j=i+1;j<n;j++)
			add_edge(i,j,Kn);

	auto edgei_map = get( boost::edge_index, Kn);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(auto [ei,ei_end] = edges(Kn);ei!=ei_end;ei++)
		put(edgei_map,*ei,ecount++);

	return Kn;
}

/**
 * Generates a cycle of size n.
 */
template <typename Graph>
Graph genCycle(int n) noexcept{

	Graph cycle(n);

	for (int i=0; i<n;i++)
		add_edge(i,(i+1) % n,cycle);

	auto edgei_map = get( boost::edge_index, cycle);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(auto [ei,ei_end] = edges(cycle);ei!=ei_end;ei++)
		put(edgei_map,*ei,ecount++);

	return cycle;
}

template <typename Graph>
Graph genV2n(int n) noexcept{

	Graph V = genCycle<Graph>(2*n);

	for (int i=0; i<=n;i++)
		add_edge(i,i+n,V);

	auto edgei_map = get( boost::edge_index, V);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(auto [ei,ei_end] = edges(V);ei!=ei_end;ei++)
		put(edgei_map,*ei,ecount++);

	return V;
}


/**
 * Generates a complete bipartite graph with parts of size p and q.
 */
template <typename Graph> 
Graph getKpq(int p,int q) noexcept
{

	Graph Kpq(p+q);

	for (int i=0; i<p;i++)
		for(int j=p;j<p+q;j++)
			add_edge(i,j,Kpq);

	auto edgei_map = get( boost::edge_index, Kpq);
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(auto [ei,ei_end] = edges(Kpq);ei!=ei_end;ei++)
		put(edgei_map,*ei,ecount++);

	return Kpq;
}

}
