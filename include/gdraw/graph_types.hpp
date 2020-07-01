#ifndef GRAPH_TYPES
#define GRAPH_TYPES

template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

template <typename Graph>	
using rotations_t = typename std::vector< std::vector< edge_t<Graph> > >;

#endif
