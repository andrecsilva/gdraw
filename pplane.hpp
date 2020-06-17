#include <iostream>
#include <vector>
#include <map>
#include <limits>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_dfs.hpp>

//#define DEBUG

#ifdef DEBUG 
#define DEBUG(x) x
#else 
#define DEBUG(x) do{}while(0);
#endif

template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

//Type for embedding scheme
//Maybe edge_t instead of std::pair?
//template <typename Graph>
using embedding_t = typename std::vector<std::vector<std::pair<size_t,int>>>;

//Reads embeddings from Myrvold and Campbell's program
embedding_t readEmbedding(){
	size_t ngraph = 0;
	size_t nvertices = 0;

	std::cin >> ngraph;
	std::cin >> nvertices;

	embedding_t embedding {nvertices};
	
	for(size_t i=0; i< nvertices; i++){
		size_t degree=0;
		std::cin >> degree;	

		embedding.at(i) = std::vector<std::pair<size_t,int>>{degree};
		for (size_t j =0; j<degree; j++){
			size_t vertex=0;
			int signal=0;
			std::cin >> vertex >> signal;
			embedding.at(i).at(j) = std::make_pair(vertex,signal);
		}
			
	}
	return embedding;
}

//Dumb struct to hold the edges and the parents of a tree obtained from a search
template <typename Graph>
struct Tree{

	Tree() : root{}, edges{}, parent{} {};
	//TODO maybe fill parent with some invalid vertex_descriptor?
	Tree(vertex_t<Graph> root, std::size_t nvertices) : root(root), edges(), parent(nvertices,std::numeric_limits<size_t>::max()) {};

	vertex_t<Graph> root;
	std::vector<edge_t<Graph>> edges;
	std::vector<vertex_t<Graph>> parent;
};

template <typename Graph>
struct DFSTreeVisitor : public boost::dfs_visitor<>{

	//Don't like this, but it seems the visitor is copied into the dfs procedure.
	Tree<Graph>* t;

	DFSTreeVisitor() : t{} {};

	DFSTreeVisitor(Tree<Graph>& t) : t{&t} {};

	void tree_edge(const edge_t<Graph> e, const Graph& g){
		t->edges.push_back(e);
		t->parent.at(target(e,g)) = source(e,g);
		//TODO crude...
		num_edges(g);
		DEBUG(std::cout << "Inserting: " << e << std::endl;)
		DEBUG(std::cout << "Parent: " << source(e,g) << std::endl;)
		DEBUG(std::cout << "Child: " << target(e,g) << std::endl;)
	}

};

//finds the dfs forest from a given graph
template <typename Graph>
Tree<Graph> dfsTree(const Graph& g, const vertex_t<Graph>& root){

	DEBUG(std::cout << "num_vertices: " <<  num_vertices(g) << std::endl;)

	Tree<Graph> tree{root,num_vertices(g)};

	DFSTreeVisitor<Graph> vis{tree};

	//std::map<edge_t<Graph>, boost::default_color_type> edge_color;
	//auto ecmap = make_assoc_property_map(edge_color);

	std::vector<boost::default_color_type> edge_color(num_edges(g));
	auto edgei_map = get(boost::edge_index,g);
	auto ecmap = make_iterator_property_map(edge_color.begin(), edgei_map);

	undirected_dfs(g,
			boost::root_vertex(root)
			.visitor(vis)
			.edge_color_map(ecmap));

	DEBUG(std::cout << "Before return... " << std::endl;)

	DEBUG(for(auto e: tree.edges){
		std::cout <<  e << " " << std::endl;
	}
	)

	DEBUG(for(auto v: tree.parent){
		std::cout <<  v << " " << std::endl;
	}
	)

	return tree;

}

//returns a vector indexed with edge_index with the signals of the edges.
template <typename Graph>
std::vector<int> getEdgeSignals(const Graph& g, embedding_t& embedding){
	std::vector<int> signal(num_edges(g));

	auto edgei_map = get(boost::edge_index,g);

	for(size_t i =0; i< embedding.size(); i++)
		for(auto e : embedding.at(i))
			signal.at(boost::get(edgei_map,edge(i,e.first,g).first)) = e.second;

	return signal;
	
}

//returns the number of negative edges in the path from the root to the other vertices
template <typename Graph>
std::vector<std::size_t> pathFromRoot(const Graph& g, const Tree<Graph> t, const std::vector<int> esignals);

template <typename Graph>
struct DoublePlanarVisitor : public boost::dfs_visitor<>{

	Graph* h;
	size_t ogvcount;
	std::vector<int>* edges_signals;
	std::vector<int>* signal_from_root;

	DoublePlanarVisitor() : ogvcount{}, edges_signals{}, signal_from_root{} {};

	DoublePlanarVisitor(Graph& h,size_t ogvcount, std::vector<int>& edges_signals, std::vector<int>& signal_from_root) : h{&h}, ogvcount{ogvcount}, edges_signals{&edges_signals}, signal_from_root{&signal_from_root} {};

	void back_edge(const edge_t<Graph> e, const Graph& g){
		auto edgei_map = get(boost::edge_index,g);

		auto u = source(e,g);
		auto v = target(e,g);

		int esignal = edges_signals->at(boost::get(edgei_map,e));
		int cycle_signal = signal_from_root->at(u) * signal_from_root->at(v) * esignal;
		//adds an edge between pair of originals and doubles if the cycle formed by the edge is two-sided
		if(cycle_signal ==-1){
			add_edge(target(e,g)+ogvcount,source(e,g),*h);
			add_edge(target(e,g),source(e,g)+ogvcount,*h);
		}else{
			add_edge(target(e,g),source(e,g),*h);
			add_edge(target(e,g)+ogvcount,source(e,g)+ogvcount,*h);
		}
		DEBUG(std::cout << "Back_edge: " << e << std::endl;)
		DEBUG(std::cout << "Edge Signal:" << edges_signals->at(boost::get(edgei_map,e)) << std::endl;)
		DEBUG(std::cout << "Signal Parent: " << signal_from_root->at(source(e,g)) << std::endl;)
		DEBUG(std::cout << "Signal Child: " << signal_from_root->at(target(e,g)) << std::endl;)
		DEBUG(std::cout << "Cycle Signal: " << cycle_signal << std::endl;)
	}

	void tree_edge(const edge_t<Graph> e, const Graph& g){
		//adds an edges between their doubles
		add_edge(target(e,g)+ogvcount,source(e,g)+ogvcount,*h);
		add_edge(target(e,g),source(e,g),*h);

		//TODO at what cost?
		auto edgei_map = get(boost::edge_index,g);
		signal_from_root->at(target(e,g)) = signal_from_root->at(source(e,g)) * edges_signals->at(boost::get(edgei_map,e));
		
		DEBUG(std::cout << "Tree_edge: " << e << std::endl;)
		DEBUG(std::cout << "Edge Signal:" << edges_signals->at(boost::get(edgei_map,e)) << std::endl;)
		DEBUG(std::cout << "Signal Parent: " << signal_from_root->at(source(e,g)) << std::endl;)
		DEBUG(std::cout << "Signal Child: " << signal_from_root->at(target(e,g)) << std::endl;)
	}

};

//TODO const the vector
template <typename Graph>
Graph planarDoubleCover(Graph& g, std::vector<int> edges_signals){

	size_t ogvcount = num_vertices(g);

	std::vector<int> signal_from_root(ogvcount,1);
	Graph h(ogvcount*2);

	DoublePlanarVisitor<Graph> vis(h,ogvcount,edges_signals,signal_from_root);

	std::vector<boost::default_color_type> edge_color(num_edges(g));
	auto edgei_map = get(boost::edge_index,g);
	auto ecmap = make_iterator_property_map(edge_color.begin(), edgei_map);

	undirected_dfs(g,
			boost::root_vertex(0)
			.visitor(vis)
			.edge_color_map(ecmap));


	auto edgei_map_h = get( boost::edge_index, h);

	//TODO maybe preserve the edge indexes of the original graph?
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(ei = edges(h).first; ei!=edges(h).second; ++ei)
		put(edgei_map_h,*ei,ecount++);

	return h;
}
