#pragma once

#include <vector>
#include <map>
#include <ranges>
#include <memory>
#include <iostream>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

#include <gdraw/coordinates.hpp>

namespace gdraw{
	
template <typename Graph>	
using edge_t = typename boost::graph_traits<Graph>::edge_descriptor;

template <typename Graph>	
using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

template <typename Graph>	
using rotations_t = typename std::vector< std::vector< edge_t<Graph> > >;

template <typename T,typename Graph>
concept EdgeRange = std::ranges::common_range<T> && std::is_same<std::remove_cvref_t<std::ranges::range_reference_t<T>>,edge_t<Graph>>::value;

template <typename T,typename Graph>
concept VertexRange = std::ranges::common_range<T> && std::is_same<std::remove_cvref_t<std::ranges::range_reference_t<T>>,vertex_t<Graph>>::value;

constexpr bool debug = false;

namespace detail{

	template <typename Graph>
	inline auto edges_iterator(Graph& g){
		return edges(g);
	}

	template <typename Graph>
	inline auto vertices_iterator(Graph& g){
		return vertices(g);
	}

	template <typename Graph>
	inline auto boost_edge(Graph& g,vertex_t<Graph> u, vertex_t<Graph> v){
		return edge(u,v,g);
	}

	template <typename Graph>
	inline auto boost_vertex(Graph& g,size_t n){
		return vertex(n,g);
	}

}

/**
 * Auxiliary function that transforms the iterator pair from boost::edges into a proper viewable range.
 *
 * Mostly used internally.
 */
template <typename BeginIterator,typename EndIterator>
inline auto range(std::pair<BeginIterator,EndIterator>&& ipair){
	return std::ranges::subrange(ipair.first,ipair.second);
}

/**
 * Auxiliary function that transforms a range into a viewable range.
 *
 * Mostly used internally.
 */
template <typename Range>
inline auto range(Range&& r){
	return std::ranges::subrange(r.begin(),r.end());
}

/**
 * Returns the pair of endpoints of a Boost Graph.
 *
 * Mostly used internally.
 */
template <typename Graph>
inline auto endpoints(const Graph& g,const edge_t<Graph>& e){
	return std::make_tuple(source(e,g),target(e,g));
}

/**
 * Takes a collection of optional elements and returns a view over
 * all the actual elements of a collection.
 */
template <template<typename> typename T,typename U>
requires std::ranges::range<T<U>>
auto inline filterOptional(const T<std::optional<U>>& collection){
	return collection | 
		std::views::filter([](auto e){return bool(e);}) | 
		std::views::transform([](auto e){return e.value();});
}

/**
 * This class wraps a boost graph class and implements move semantics using std::unique_ptr.
 *
 * The reason for the existence of this class is that Boost Graph objects do not play well with move semantics.
 *
 */
template <typename Graph>
class GraphWrapper{
	private:
		std::unique_ptr<Graph> base_graph;
	public:

		GraphWrapper(const Graph& base){
			base_graph = std::make_unique<Graph>(base);
		}

		GraphWrapper(const GraphWrapper& other){
			//base_graph = std::make_unique<Graph>(*other.base_graph);
			if constexpr (debug)
			       	std::cout << "GraphWrapper copy constructor" << std::endl;
			*this = other;
		}

		GraphWrapper(GraphWrapper&& other){
			if constexpr (debug)
			       	std::cout << "GraphWrapper move constructor" << std::endl;
		
			*this = std::move(other);
			//base_graph = std::move(other.base_graph);
			//other.base_graph = std::make_unique<Graph>();
		}

		GraphWrapper& operator=(GraphWrapper&& other){
			if constexpr (debug)
				std::cout << "GraphWrapper move assignment" << std::endl;
			base_graph = std::make_unique<Graph>();
			base_graph.swap(other.base_graph);
			return *this;
		}

		GraphWrapper& operator=(const GraphWrapper& other){
			if constexpr (debug)
				std::cout << "GraphWrapper copy assignment" << std::endl;
			base_graph = std::make_unique<Graph>(*other.base_graph);
			//base_graph = std::move(other.base_graph);
			return *this;
		}
		//TODO Ideally I'd like to make a implicity conversion
		//TODO Hard to make it work with boost
		//operator Graph&(){
		//	return *base_graph;
		//}

		inline Graph& getGraph(){
			return *base_graph;
		}

		inline Graph& getGraph() const{
			return *base_graph;
		}

};

/**
 * An adaptor around a boost graph with indexes as an internal property for both edges
 * and vertices. The methods of this class will take care of the indexing of the edges
 * for you.
 */
template <typename Graph>
class IndexedGraph : public GraphWrapper<Graph>{
	public:
		boost::property_map<Graph, boost::vertex_index_t>::type vertexi_map;
		boost::property_map<Graph, boost::edge_index_t>::type edgei_map;

		IndexedGraph(Graph g) 
		: GraphWrapper<Graph>(std::move(g))
		{
			vertexi_map = get( boost::vertex_index, g);
			edgei_map = get( boost::edge_index, g);
		}

		//IndexedGraph(IndexedGraphDecorator<Graph>& dg){
		//	this = dg.g;
		//}

		IndexedGraph(const IndexedGraph<Graph>& other) 
		: GraphWrapper<Graph>(other)
		{
			if constexpr (debug)
				std::cout << "IndexedGraph copy constructor" << std::endl;
			vertexi_map = get( boost::vertex_index, this->getGraph());
			edgei_map = get( boost::edge_index, this->getGraph());
		}

		IndexedGraph(IndexedGraph&& other)
		: GraphWrapper<Graph>(std::move(other)){
			if constexpr (debug)
				std::cout << "IndexedGraph move constructor" << std::endl;
			vertexi_map = get( boost::vertex_index, this->getGraph());
			edgei_map = get( boost::edge_index, this->getGraph());
		}

		IndexedGraph& operator=(IndexedGraph&& other){
			GraphWrapper<Graph>::operator=(std::move(other));
			if constexpr (debug)
				std::cout << "IndexedGraph move assignment" << std::endl;
			vertexi_map = get( boost::vertex_index, this->getGraph());
			edgei_map = get( boost::edge_index, this->getGraph());
			return *this;
		}

		IndexedGraph& operator=(const IndexedGraph& other){
			GraphWrapper<Graph>::operator=(other);
			if constexpr (debug)
				std::cout << "IndexedGraph copy assignment" << std::endl;
			(*this) = IndexedGraph(other);
			return *this;
		}

		inline auto index(edge_t<Graph> e) const{
			return boost::get(edgei_map,e);
		}

		inline auto index(vertex_t<Graph> v) const{
			return boost::get(vertexi_map,v);
		}

		auto changeIndex(edge_t<Graph> e, auto i) const -> void{
			boost::put(edgei_map,e,i);
		}

		auto addEdge(vertex_t<Graph> u, vertex_t<Graph> v, auto i) -> edge_t<Graph>{
			auto e = add_edge(u,v,this->getGraph()).first;
			boost::put(edgei_map,e,i);
			return e;
		}

		auto addEdge(vertex_t<Graph> u, vertex_t<Graph> v) -> edge_t<Graph>{
			auto e = add_edge(u,v,this->getGraph()).first;
			boost::put(edgei_map,e,num_edges(this->getGraph())-1);
			return e;
		}

		auto removeEdge(edge_t<Graph> e) -> void{ 
			remove_edge(e,this->getGraph());
		}

		auto addVertex() -> vertex_t<Graph>{
			return add_vertex(this->getGraph());
		}

		inline auto edges() const{
			return range(detail::edges_iterator(this->getGraph()));
		}

		inline auto vertices() const{
			return range(detail::vertices_iterator(this->getGraph()));
		}

		inline auto incidentEdges(vertex_t<Graph> v) const{
			return range(out_edges(v,this->getGraph()));
		}

		inline auto neighbors(vertex_t<Graph> v) const{
			return range(adjacent_vertices(v,this->getGraph()));
		}

		inline auto numEdges() const{
			return num_edges(this->getGraph());
		}

		inline auto numVertices() const{
			return num_vertices(this->getGraph());
		}

		inline auto degree(vertex_t<Graph> v) const{
			return out_degree(v,this->getGraph());
		}

		inline auto endpoints(edge_t<Graph> e) const{
			return std::make_tuple(source(e,this->getGraph()),target(e,this->getGraph()));
		}

		inline auto edge(vertex_t<Graph> u, vertex_t<Graph> v) -> std::optional<edge_t<Graph>>{
			auto e = detail::boost_edge(this->getGraph(),u,v);
			if(e.second)
				return e.first;
			return {};
		}

		inline auto vertex(size_t n) -> vertex_t<Graph>{
			return detail::boost_vertex(this->getGraph(),n);
		}
		
		inline static auto nullVertex(){
			return boost::graph_traits<Graph>::null_vertex();
		}

};

/**
 * A base class for decorating IndexedGraphs. It should implement all 
 * methods of IndexedGraph.
 */
template <typename Graph>
class IndexedGraphDecorator{

	public:
		IndexedGraph<Graph>& graph;

		IndexedGraphDecorator(IndexedGraph<Graph>& graph) : graph(graph){
			if constexpr(debug){
				std::cout << "IndexedGraphDecorator constructor" << std::endl;
			}
		}

		inline auto index(edge_t<Graph> e) const{
			return graph.index(e);
		}

		inline auto index(vertex_t<Graph> v) const{
			return graph.index(v);
		}

		auto changeIndex(edge_t<Graph> e, auto i) const -> void{
			graph.changeIndex(e,i);
		}

		auto addEdge(vertex_t<Graph> u, vertex_t<Graph> v, auto i) -> edge_t<Graph>{
			return graph.addEdge(u,v,i);
		}

		auto addEdge(vertex_t<Graph> u, vertex_t<Graph> v) -> edge_t<Graph>{
			return graph.addEdge(u,v);
		}

		auto removeEdge(edge_t<Graph> e) -> void{ 
			graph.removeEdge(e);
		}

		auto addVertex() -> vertex_t<Graph>{
			return graph.addVertex();
		}

		inline auto edges() const{
			return graph.edges();
		}

		inline auto vertices() const{
			return graph.vertices();
		}

		inline auto incidentEdges(vertex_t<Graph> v) const{
			return graph.incidentEdges(v);
		}

		inline auto neighbors(vertex_t<Graph> v) const{
			return graph.neighbors();
		}

		inline auto numEdges() const{
			return graph.numEdges();
		}

		inline auto numVertices() const{
			return graph.numVertices();
		}

		inline auto degree(vertex_t<Graph> v) const{
			return graph.degree(v);
		}

		inline auto endpoints(edge_t<Graph> e) const{
			return graph.endpoints(e);
		}

		inline auto edge(vertex_t<Graph> u, vertex_t<Graph> v) -> std::optional<edge_t<Graph>>{
			return graph.edge(u,v);
		}

		inline auto vertex(size_t n) -> vertex_t<Graph>{
			return graph.vertex(n);
		}
		
		inline static auto nullVertex(){
			return IndexedGraph<Graph>::nullVertex();
		}

		inline Graph& getGraph(){
			return this->graph.getGraph();
		}

		inline Graph& getGraph() const{
			return this->graph.getGraph();
		}

		//operator IndexedGraph<Graph>&(){
		//	return graph;
		//}

		//operator const IndexedGraph<Graph>&() const{
		//	return graph;
		//}
};

//template <template <typename> typename S,typename Graph>
//concept AsIndexedGraph = std::is_same<std::remove_cvref_t<S<Graph>>,IndexedGraph<Graph>>::value ||
//		std::is_same<std::remove_cvref_t<S<Graph>>,IndexedGraphDecorator<Graph>>::value;
//
template <template <typename> typename S,typename Graph>
concept AsIndexedGraph = std::is_base_of<IndexedGraph<Graph>,std::remove_cvref_t<S<Graph>>>::value ||
		std::is_base_of<IndexedGraphDecorator<Graph>,std::remove_cvref_t<S<Graph>>>::value;


/**
 * Constructs and maintain edge list for the given IndexedGraph. 
 */
template <typename Graph>
class EdgeList : public IndexedGraphDecorator<Graph>{

	public:
		std::vector<edge_t<Graph>> edges_by_index;

		boost::graph_traits<Graph>::edges_size_type ecount;

		EdgeList(IndexedGraph<Graph>& graph) : IndexedGraphDecorator<Graph>(graph){

			edges_by_index = std::vector<edge_t<Graph>>(graph.numEdges());

			for(auto&& e : graph.edges()){
				edges_by_index[graph.index(e)] = e;
			}

			ecount = graph.numEdges();
			//std::cout << ecount << std::endl;

		}

		auto addEdge(vertex_t<Graph> u, vertex_t<Graph> v) -> edge_t<Graph>{
			if(ecount < edges_by_index.size()){
				auto e = this->graph.addEdge(u,v,ecount);
				edges_by_index[ecount] = e;
				ecount++;
				return e;
			}else{
				auto e = this->graph.addEdge(u,v,ecount);
				edges_by_index.push_back(e);
				ecount++;
				return e;
			}
		}

		auto addEdge(vertex_t<Graph> u, vertex_t<Graph> v, auto i) -> edge_t<Graph>{
			auto e = this->graph.addEdge(u,v,i);
			edges_by_index[i] = e;
			return e;
		}

		auto removeEdge(edge_t<Graph> e) -> void{
			auto e_index = this->graph.index(e);
			auto f = edges_by_index[ecount-1];
			edges_by_index[e_index] = f;
			this->graph.changeIndex(f,e_index);
			remove_edge(e,this->graph.getGraph());
			ecount--;
		}

		inline auto edge(auto i) -> edge_t<Graph>{
			return edges_by_index[i];
		}

		auto print() -> void{
			for(auto&& e : edges_by_index)
				std::cout << e << '[' << this->graph.index(e) << ']' << ' ';
			std::cout << std::endl;
		}

};

/**
 * Contains a vertex and edge map from one IndexedGraph to another.
 * For an edge e of graph, map(e) contains the edge of the supergraph
 * it is mapped to.
 */
template <typename Graph>
class SubgraphMap: public IndexedGraphDecorator<Graph>{

	public:
		std::vector<vertex_t<Graph>> ivertex_map;
		std::vector<edge_t<Graph>> iedge_map;

		SubgraphMap(IndexedGraph<Graph>& from,
				std::vector<vertex_t<Graph>> ivertex_map,
				std::vector<edge_t<Graph>> iedge_map):
			IndexedGraphDecorator<Graph>{from},
			ivertex_map{std::move(ivertex_map)},
			iedge_map{std::move(iedge_map)}{}
		

		inline auto map(edge_t<Graph> e) -> edge_t<Graph>{
			return iedge_map[this->graph.index(e)];
		}

		inline auto map(vertex_t<Graph> v) -> vertex_t<Graph>{
			return ivertex_map[this->graph.index(v)];
		}

};

template <template <typename> typename T,typename Graph>
concept AsGraphWrapper = std::convertible_to<T<Graph>,GraphWrapper<Graph>>;

/**
 * An abstract class dealing with the internals of rotations.
 * You probaly do not want to use this.
 */
template <typename Graph>
class PureEmbeddedGraph : public IndexedGraph<Graph>{
	public:
		rotations_t<Graph> rotations;

		PureEmbeddedGraph(IndexedGraph<Graph> g, rotations_t<Graph> rotations)
		: IndexedGraph<Graph>(std::move(g)),
		rotations(std::move(rotations)){}

		PureEmbeddedGraph(PureEmbeddedGraph&& other) : IndexedGraph<Graph>(std::move(other)){
			if constexpr(debug)
				std::cout << "PureEmbeddedGraph Move build" << std::endl;
			(*this).rotations = std::move(other.rotations);
		}

		PureEmbeddedGraph(const PureEmbeddedGraph& other) : IndexedGraph<Graph>(other){
			if constexpr(debug)
				std::cout << "PureEmbeddedGraph Copy build" << std::endl;
			(*this).rotations = rotations_t<Graph>(num_vertices(other.getGraph()));
					
			auto this_edgei_map = get(boost::edge_index, (*this).getGraph());
			auto other_edgei_map = get(boost::edge_index, other.getGraph());

			std::vector<edge_t<Graph>> edges_by_index(num_edges((*this).getGraph()));

			//TODO where is edges defined?
			for(auto e : range(edges((*this).getGraph()))){
				auto ei = boost::get(this_edgei_map,e);
				edges_by_index[ei] = e;
			}
			
			//for(auto&& pi_u : other.rotations){
			//	for(auto&& e : pi_u){
			//		auto [u,v] = endpoints(e);
			//		auto f = edge(u,v,other.getGraph()).first;
			//		std::cout << e << '[' << get(other_edgei_map,e)  << ',' << get(other_edgei_map,f) <<  ']' << ' ' ;
			//	}
			//	std::cout << std::endl;
			//}

			for(auto v =0;auto&& pi_v : other.rotations){
				for(auto&& e : pi_v){
					auto ei = boost::get(other_edgei_map,e);
					auto f = edges_by_index[ei];
					rotations[v].push_back(f);
				}
				v++;
			}
		}
		
		PureEmbeddedGraph& operator=(PureEmbeddedGraph&& other){
			if constexpr(debug)
				std::cout << "PureEmbeddedGraph Move =" << std::endl;
			IndexedGraph<Graph>::operator=(std::move(other));
			(*this).rotations = std::move(other.rotations);
			return *this;
		}

		PureEmbeddedGraph& operator=(const PureEmbeddedGraph& other){ 
			if constexpr(debug)
				std::cout << "Copy =" << std::endl;
			(*this) = PureEmbeddedGraph(other);
			return *this;
		}

};

/*
 * A graph with an associated embedding scheme. 
 */
template <typename Graph>
class EmbeddedGraph : public PureEmbeddedGraph<Graph>{

	public:
		std::vector<int> edge_signals;

		EmbeddedGraph(IndexedGraph<Graph> g, rotations_t<Graph> rotations, std::vector<int> edge_signals):
			PureEmbeddedGraph<Graph>{std::move(g),std::move(rotations)},
			edge_signals(std::move(edge_signals)){}

		inline auto signal(edge_t<Graph> e) const -> int{
			auto edgei_map = get(boost::edge_index, this->getGraph());
			return edge_signals[get(edgei_map,e)];
		}
};

/*
 * A Graph that is embedded in an orientable surface with EulerGenus `EulerGenus`.
 * It assumes that the all the edges have positive signals.
 */
template <typename Graph, int EulerGenus>
class OrientableEmbeddedGraph : public EmbeddedGraph<Graph>{

	public:

		OrientableEmbeddedGraph(IndexedGraph<Graph> g, rotations_t<Graph> rotations,std::vector<int> edge_signals):
			EmbeddedGraph<Graph>{std::move(g),std::move(rotations),std::move(edge_signals)}{}
			//,euler_characteristic(std::move(euler_characteristic)) {}
			
		OrientableEmbeddedGraph(IndexedGraph<Graph> g, rotations_t<Graph> rotations):
			EmbeddedGraph<Graph>{std::move(g),std::move(rotations),std::vector<int>(num_edges(g.getGraph()))} {}

		inline auto signal(edge_t<Graph>) const -> int{
			return 1;
		}
};

/*
 * A Graph that is embedded in a non-orientable surface with EulerGenus `EulerGenus`.
 */
template <typename Graph, int EulerGenus>
class NonOrientableEmbeddedGraph : public EmbeddedGraph<Graph>{

	public:
		NonOrientableEmbeddedGraph(IndexedGraph<Graph> g, rotations_t<Graph> rotations,std::vector<int> edge_signals):
			EmbeddedGraph<Graph>{std::move(g),std::move(rotations),std::move(edge_signals)}{}

		inline auto genus() -> int{
			return EulerGenus;
		}

};

//TODO include the genus template parameter on this class
/** 
 * A graph together with a list of edges that induces a forbidden subgraph.
 */
template <typename Graph>
class NonEmbeddableGraph : public IndexedGraph<Graph>{

	public:
		std::vector<edge_t<Graph>> forbidden_subgraph;

		NonEmbeddableGraph(IndexedGraph<Graph> g, std::vector<edge_t<Graph>> forbidden_subgraph):
			IndexedGraph<Graph>(std::move(g)),
			forbidden_subgraph(std::move(forbidden_subgraph)) {}

		NonEmbeddableGraph(NonEmbeddableGraph&& other) : IndexedGraph<Graph>(std::move(other)), forbidden_subgraph(std::move(other.forbidden_subgraph)){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Move Constructor" << std::endl;
		}

		NonEmbeddableGraph(const NonEmbeddableGraph& other) : IndexedGraph<Graph>(other){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Copy Constructor" << std::endl;
			auto this_edgei_map = get(boost::edge_index, (*this).getGraph());
			auto other_edgei_map = get(boost::edge_index, other.getGraph());

			std::vector<edge_t<Graph>> edges_by_index(num_edges((*this).getGraph()));

			//TODO where is edges defined?
			for(auto e : range(edges((*this).getGraph()))){
				auto ei = boost::get(this_edgei_map,e);
				edges_by_index[ei] = e;
			}

			for(auto&& e : other.forbidden_subgraph){
				auto ei = boost::get(other_edgei_map,e);
				(*this).forbidden_subgraph.push_back(edges_by_index[ei]);
			}

		}


		NonEmbeddableGraph& operator=(NonEmbeddableGraph&& other){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Move =" << std::endl;
			IndexedGraph<Graph>::operator=(std::move(other));
			(*this).forbidden_subgraph = std::move(other.forbidden_subgraph);
			return *this;
		}

		NonEmbeddableGraph& operator=(const NonEmbeddableGraph& other){
			if constexpr(debug)
				std::cout << "NonEmbeddableGraph Copy =" << std::endl;
			*this = NonEmbeddableGraph(other);
			return *this;
		}
};

/**
 * A graph drawn on the plane.
 *
 * Edges are drawn as cubic splines and are stored on `edge_coordinates`. Edges whose coordinates are not include there should be interpreted as a line segment between its endpoints.
 */
template <typename Graph>
class DrawnGraph : public IndexedGraph<Graph>{

	public:
		std::vector<coord_t> coordinates;
		std::vector<std::string> vertex_colors;
		std::map<edge_t<Graph>,cubicSpline> edge_coordinates;
		std::map<edge_t<Graph>,std::string> edge_colors;

		DrawnGraph(IndexedGraph<Graph> g, std::vector<coord_t> coordinates):
			IndexedGraph<Graph>(std::move(g)),
			coordinates(std::move(coordinates)),
       			vertex_colors(num_vertices((*this).getGraph()),""){
				//std::cout << "Constructor DrawnGraph" << std::endl;
				//std::cout << "Size: " << vertex_colors.size() << std::endl;
				//std::cout << "Num_vertices: " << num_vertices(this->getGraph()) << std::endl;
			}

		DrawnGraph(IndexedGraph<Graph> g, std::vector<coord_t> coordinates,std::map<edge_t<Graph>,cubicSpline> edge_coordinates):
			IndexedGraph<Graph>(std::move(g)),
			coordinates(std::move(coordinates)),
       			vertex_colors(num_vertices((*this).getGraph()),""),
			edge_coordinates(std::move(edge_coordinates)){}

		DrawnGraph(DrawnGraph&& other):
		       	IndexedGraph<Graph>(std::move(other)),
       			vertex_colors(std::move(other.vertex_colors)),
			edge_coordinates(std::move(other.edge_coordinates)),
			edge_colors(std::move(other.edge_colors)){
				if constexpr(debug)
					std::cout << "DrawnGraph Move constructor" << std::endl;
			}

		DrawnGraph(const DrawnGraph& other) : IndexedGraph<Graph>(other){
			if constexpr(debug)
				std::cout << "DrawnGraph Copy constructor" << std::endl;
			coordinates = other.coordinates;
			vertex_colors = other.vertex_colors;

			auto this_edgei_map = get(boost::edge_index, (*this).getGraph());
			auto other_edgei_map = get(boost::edge_index, other.getGraph());

			std::vector<edge_t<Graph>> edges_by_index(num_edges((*this).getGraph()));

			//TODO where is edges defined?
			for(auto e : range(edges((*this).getGraph()))){
				auto ei = boost::get(this_edgei_map,e);
				edges_by_index[ei] = e;
			}

			for(auto&& [e,s] : other.edge_coordinates){
				auto ei = boost::get(other_edgei_map,e);
				auto f = edges_by_index[ei];
				std::cout << '[' << ei << ']' << e << ' ' << f << ' ' << s << std::endl;
				(*this).edge_coordinates[f] = s;
			}

			for(auto&& [e,c] : other.edge_colors){
				auto ei = boost::get(other_edgei_map,e);
				auto f = edges_by_index[ei];
				(*this).edge_colors[f] = c;
			}
		}

		DrawnGraph& operator=(DrawnGraph&& other){
			if constexpr(debug)
				std::cout << "DrawnGraph Move =" << std::endl;
			IndexedGraph<Graph>::operator=(std::move(other));
			(*this).edge_colors = std::move(other.edge_colors);
			(*this).edge_coordinates = std::move(other.edge_coordinates);
			(*this).vertex_colors = std::move(other.vertex_colors);
			(*this).coordinates = std::move(other.coordinates);
			return *this;
		}

		DrawnGraph& operator=(const DrawnGraph& other){
			if constexpr(debug)
				std::cout << "DrawnGraph Copy =" << std::endl;
			*this = DrawnGraph(other);
			return *this;
		}

		auto colorEdge(const edge_t<Graph>& e,std::string color){
			edge_colors[e] = std::move(color);

		}

		auto colorVertex(const vertex_t<Graph>& v, std::string color){
			//TODO maybe change this to explicitly use vertex_index map
			vertex_colors[v] = std::move(color);
		}
};

template <typename Graph>
using PlanarGraph = OrientableEmbeddedGraph<Graph,0>;

template <typename Graph>
using NonPlanarGraph = NonEmbeddableGraph<Graph>;

template <typename Graph>
using ProjectivePlanarGraph = NonOrientableEmbeddedGraph<Graph,2>;

/**
 * Returns a copy of g and g_edges such that the copy of g_edges are valid descriptors in the copied graph.
 *
 * Mostly used for debugging purposes or internally.
 * @return: a tuple containing a `IndexedGraph` and a `EdgeRange`
 */
auto graph_copy(const auto& g, const auto& g_edges){
	//decltype(g) h = g;
	auto h = g;

	using edge_t = typename std::remove_cvref<decltype(*(edges(h.getGraph()).first))>::type;
	//std::cout << typeid(edge_t).name() << std::endl;
	std::vector<edge_t> h_edges{};

	auto g_edgei_map = get(boost::edge_index, g.getGraph());
	auto h_edgei_map = get(boost::edge_index, h.getGraph());

	std::vector<edge_t> edges_by_index(num_edges(h.getGraph()));

	for(auto&& e : range(edges(h.getGraph()))){
		auto ei = boost::get(h_edgei_map,e);
		edges_by_index[ei] = e;
	}

	for(auto&& e : g_edges){
		auto ei = boost::get(g_edgei_map,e);
		h_edges.push_back(edges_by_index[ei]);
	}

	return std::make_tuple(std::move(h),std::move(h_edges));
}

/** 
 * Lists all the vertices and edges of a graph with its indexes.
 */
template <template <typename> typename S,typename Graph>
requires AsIndexedGraph<S,Graph>
void printGraph(const S<Graph>& g){
	std::cout << "Vertices: " << std::endl;
	for(auto&& v : range(vertices(g.getGraph())))
		std::cout << v << ' ';
	std::cout << std::endl;

	auto edgei_map = get( boost::edge_index, g.getGraph());

	std::cout << "Edges: " << std::endl;
	for(auto&& e : range(edges(g.getGraph())))
		std::cout << "[" << boost::get(edgei_map,e) << "]" << e << " "; 

	std::cout << std::endl;
}

/**
 * Prints the embedding scheme of an embedded graph.
 */
template <typename Graph>
auto printEmbedding(const EmbeddedGraph<Graph>& g){
	for(auto&& pi_u : g.rotations){
		for(auto&& e : pi_u){
			std::cout << (g.signal(e)==1?'+':'-') << e << ' ';
		}
		std::cout << std::endl;
	}
}

}//namespace
