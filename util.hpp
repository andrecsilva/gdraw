#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

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

