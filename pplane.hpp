#include <iostream>
#include <vector>
#include <map>
#include <limits>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

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

template <typename Graph>	
using rotations_t = typename std::vector< std::vector< edge_t<Graph> > >;

template <typename Graph>
using edge_signals_t = typename boost::iterator_property_map<
	std::vector<int>::iterator,
       	typename boost::property_map<Graph,boost::edge_index_t>::type>;

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
	auto edgei_map = boost::get(boost::edge_index,g);
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

	auto edgei_map = boost::get(boost::edge_index,g);

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
		auto edgei_map = boost::get(boost::edge_index,g);

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
		auto edgei_map = boost::get(boost::edge_index,g);
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
	auto edgei_map = boost::get(boost::edge_index,g);
	auto ecmap = make_iterator_property_map(edge_color.begin(), edgei_map);

	undirected_dfs(g,
			boost::root_vertex(0)
			.visitor(vis)
			.edge_color_map(ecmap));


	auto edgei_map_h = boost::get( boost::edge_index, h);

	//TODO maybe preserve the edge indexes of the original graph?
	typename boost::graph_traits<Graph>::edges_size_type ecount = 0;

	typename boost::graph_traits<Graph>::edge_iterator ei;

	for(ei = edges(h).first; ei!=edges(h).second; ++ei)
		boost::put(edgei_map_h,*ei,ecount++);

	return h;
}

/*
 * Returns a double cover of g from a collection of edges.
 * TODO return the projection?
 */
template <typename Graph, template<typename> typename EdgeContainer,typename Edge>
Graph doubleCover(const Graph& g, const EdgeContainer<Edge>& xedges){
	//copy graph with edges;
	Graph h(g);

	auto n = num_vertices(g);

	for(size_t i=0;i<n;i++)
		add_vertex(h);

	auto h_ecount = num_edges(h);
	auto edgei_map = boost::get(boost::edge_index,h);

	//Adds more edges than it should, but they are removed
	for(auto [ei,ei_end] = edges(g); ei!=ei_end;ei++){
		auto u = target(*ei,g);
		auto v = source(*ei,g);
		auto f = add_edge(vertex(u+n,h),vertex(v+n,h),h).first;
		boost::put(edgei_map,f,h_ecount++);
	}
					

	for(auto e: xedges){
		auto u = target(e,g);
		auto v = source(e,g);
		remove_edge(u,v,h);
		remove_edge(vertex(u+n,h),vertex(v+n,h),h);
		auto f1 = add_edge(u,vertex(v+n,h),h).first;
		auto f2 = add_edge(vertex(u+n,h),v,h).first;
		boost::put(edgei_map,f1,boost::get(edgei_map,e));
		boost::put(edgei_map,f2,h_ecount++);
	}
		
	return h;
}

template <typename Graph>
std::vector<edge_t<Graph>> getCrossEdges(Graph& dpc,size_t inv){

	std::vector<edge_t<Graph>> xedges;

	for(auto[ei,ei_end]= edges(dpc); ei!=ei_end;ei++){
		auto u = source(*ei,dpc);
		auto v = target(*ei,dpc);
		if((u < inv && v >=inv) ||
		(v < inv && u >=inv  ))
			xedges.push_back(*ei);
	}

	return xedges;

}

/*
 * Returns an embedding of G in the Projective Plane from a
 * double planar cover.
 * TODO const the arguments? problems with the property maps...
 */
template <typename Graph>
std::tuple<rotations_t<Graph>,std::vector<int>>
embeddingFromDPC(Graph& g, Graph& dpc){



	rotations_t<Graph> embedding(num_vertices(g));

	rotations_t<Graph> embedding_dpc(num_vertices(dpc));

	boyer_myrvold_planarity_test(
			boost::boyer_myrvold_params::graph = dpc
			,boost::boyer_myrvold_params::embedding = &embedding_dpc[0]
			);

	//for(auto u : embedding_dpc){
	//	for(auto e : u)
	//		std::cout << e << " ";
	//	std::cout << std::endl;
	//}
	//	std::cout << std::endl;

	auto n = num_vertices(g);

	std::vector<edge_t<Graph>> xedges = getCrossEdges(dpc,n);

	auto edgei_map = boost::get(boost::edge_index,g);

//	std::vector<bool> in_xedges(num_edges(h),false);
//
//	auto in_xedges_map = make_iterator_property_map(in_xedges.begin(), edgei_map);
//
//
//	for(auto e : xedges){
//		put(in_xedges_map,e,true);
//	}
//
	std::vector<int> edge_signals(num_edges(g),1);
	//edge_signals_t<Graph> edge_signals_map (edge_signals.begin(),edgei_map);
	edge_signals_t<Graph> edge_signals_map (edge_signals.begin(),edgei_map);

	for(size_t i =0; i<n; i++){
		for(auto e : embedding_dpc[i]){
			auto u = target(e,dpc);
			auto v = source(e,dpc);
			int signal = 1;
			if(u < n && v >=n){
				v-=n;
				signal = -1;
			}
			if(v < n && u >=n){
				u-=n;
				signal = -1;
			}
			auto f = edge(u,v,g).first;
			//std::cout << "[" << boost::get(edgei_map,f) << "]" << f << " " << signal << std::endl;
			embedding[i].push_back(f);
			boost::put(edge_signals_map,f,signal);
		}
	}

	//for(auto [ei,ei_end] = edges(g); ei!=ei_end; ++ei){
	//	std::cout << "[" << boost::get(edgei_map,*ei) << "]" << *ei << " " << boost::get(edge_signals_map,*ei) << " ";
	//}

	//for (auto s : edge_signals)
	//	std::cout << s << " ";
	//std::cout << std::endl;

	return {embedding,edge_signals};
}

/*
 * Tries to find an embedding of g in the projective plane.
 * Enumerates over all subsets of the edges not in t, find
 * a double cover and test for planarity.
 */
template <typename Graph,typename Tree>
void findProjectiveEmbedding(const Graph& g, Tree& t);
