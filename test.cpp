#include <iostream>
#include <functional>
#include <utility>
#include <vector>
#include <ranges>
#include <algorithm>
#include <span>
#include <variant>
#include <type_traits>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/taus88.hpp>

#include <cppitertools/combinations.hpp>
#include <cppitertools/enumerate.hpp>

//#include "include/gdraw/iterator_pair_range.hpp"
#include <gdraw/graph_types.hpp>
#include <gdraw/util.hpp>
#include <gdraw/draw.hpp>
#include <gdraw/io.hpp>
#include <gdraw/xnumber.hpp>
#include <gdraw/generators.hpp>
#include <gdraw/pplane.hpp>


using AdjList = boost::adjacency_list<
boost::vecS
,boost::vecS
,boost::undirectedS
,boost::property<boost::vertex_index_t,size_t>
,boost::property<boost::edge_index_t,size_t>
>; 

// TODO:
// cut along operation for 1-sided cycles
// cut along for 2-sided cycles
// facial drawing for pplanar graphs
// test for separable cycle
// test for contractible cycle
// find the smallest non-contractible cycle
// 2-connected to 3-connected planar graph -> add a vertex connected to each vertex of the facial cycle (should get a bit better result with Tutte's Embedding)
// function to load an embedding from a file
// generator: graph and vertex_pair <- takes care of indexes for you
// Documentation
// Planarity test without moves
// Refactor planarxnumber and xnumber to be more modular (lots of shared code between them)
// genus: rotations_t<Graph>,VertexSignals -> int
// See about the removal of AsGraphWrapper (problems with template deduction?)
// DONE:
// xNumber: Graph,k,f -> variant<EmbeddableGraph<Graph,Genus>, NonEmbeddableGraph<Graph,Genus>a>
// planarizeXNumber: EmbeddedGraph,VertexRange -> DrawnGraph
// findDoublePlanarCover: Graph -> Maybe(Graph)
// doubleCover: const Graph -> Graph
// doublePlanarCover: Graph -> Maybe(Graph)
// findFacialCycle: const EmbeddedGraph -> Edge_Range
// makeMaximal: EmbeddedGraph -> EmbeddedGraph
// embed: Graph -> variant(NonEmbeddableGraph,EmbeddedGraph)
// draw: EmbeddedGraph -> DrawnGraph
// writeDOT: variant<Graph,DrawnGraph> -> String
// PlanarXNumber algorithm
// Move/Copy operations on DrawnGraph and NonEmbeddableGraph
// WriteDOT -> commas after pos in edges and vertices
// Debug and fix findDoublePlanarCover -> problem: we permanently modify the graph when we call doubleCover
// Check if remove_edge in the code removes a single edges only...
// Individual programs (xnumber,finddpc,findpembedding)...
// Tidy up makefile
// Unit Tests
// Readme in github
// test for 1-sided cycle

using namespace gdraw;


struct output_visitor : public boost::planar_face_traversal_visitor
{
	void begin_face() { std::cout << "New face: "; }
	void end_face() { std::cout << std::endl; }
};

struct vertex_output_visitor : public output_visitor
{
	template < typename Vertex > void next_vertex(Vertex v)
	{
		std::cout << v << " ";
	}
};
//auto graph_copy(const auto& g, const auto& g_edges){
//	//decltype(g) h = g;
//	auto h = g;
//
//	using edge_t = typename std::remove_cvref<decltype(*(edges(h.getGraph()).first))>::type;
//	std::cout << typeid(edge_t).name() << std::endl;
//	typename std::remove_cvref<decltype(g_edges)>::type h_edges{};
//
//	auto g_edgei_map = get(boost::edge_index, g.getGraph());
//	auto h_edgei_map = get(boost::edge_index, h.getGraph());
//
//	std::vector<edge_t> edges_by_index(num_edges(h.getGraph()));
//
//	for(auto&& e : range(edges(h.getGraph()))){
//		auto ei = boost::get(h_edgei_map,e);
//		edges_by_index[ei] = e;
//	}
//
//	for(auto&& e : g_edges){
//		auto ei = boost::get(g_edgei_map,e);
//		h_edges.push_back(edges_by_index[ei]);
//	}
//
//	std::cout << "Point" << std::endl;
//	return std::make_tuple(std::move(h),std::move(h_edges));
//}

/*
 * Cuts along a cycle in an embedded graph. The result is a new graph
 * where for each vertex v of the cycle is split into two vertices.
 * Their neighbors are determined by which "side" of the cycle they are.
 */
template <typename Graph, int Genus,EdgeRange<Graph> T>
auto cutAlongCycle(NonOrientableEmbeddedGraph<Graph,Genus>& g, const T& cycle){

	//maintain an edgelist by index
	auto edgei_map = get(boost::edge_index,g.getGraph());
	std::vector<edge_t<Graph>> edges_by_index(num_edges(g.getGraph()));

	for(auto e : range(edges(g.getGraph()))){
		auto ei = boost::get(edgei_map,e);
		edges_by_index[ei] = e;
	}

	//we need to buffer all the edge that will be removed to avoid
	//invalid pointers midway
	std::vector<edge_t<Graph>> to_remove;
	auto ecount = num_edges(g.getGraph());

	//finds a common endpoint between 2 edges, if any
	auto find_common = [&g](auto e,auto f){
		auto [v,w] = gdraw::endpoints(g.getGraph(),e);
		auto [x,y] = gdraw::endpoints(g.getGraph(),f);
		if(v==x || v==y)
			return v;
		if(w==x || w==y)
			return w;
		return boost::graph_traits<Graph>::null_vertex();
	};

	//grab the next edge in a cycle
	auto get_next_edge = [&cycle](auto ei){
		if(ei+1 == cycle.end())
			return *(cycle.begin());
		return *(ei+1);
	};

	//checks wheter an edge is on the left (right) side of the cycle
	auto is_left = [](auto i,auto ei,auto fi){
		if(ei< fi){
			return i<ei || i>fi;
		}
			return fi<i && i<ei;
	};

	//will contain the new rotations 
	rotations_t<Graph> new_rotations(num_vertices(g.getGraph()));

	auto e = *(cycle.begin());
	auto f = get_next_edge(cycle.begin());
	auto u = find_common(e,f);
	auto up = add_vertex(g.getGraph());
	new_rotations.push_back({});


	auto first_vertex = u;
	auto first_vertex_p = up;

	//finds the other endpoint of e that is not u
	auto find_other_endpoint = [&g](auto f,auto u){
		auto [a,b] = gdraw::endpoints(g.getGraph(),f);
		return  a!=u? a : b;
	};

	// -e- u -f- v
	// up = u' is the left "side" of u, similarly for v
	// fp = f', fdp = f'', left and right side of f w.r.t. u
	for(auto&& ei = cycle.begin();ei!=cycle.end();ei++){
		auto e = *ei;
		auto f = get_next_edge(ei);

		auto v = find_other_endpoint(f,u);
		decltype(v) vp;

		if(v!=first_vertex){
			vp = add_vertex(g.getGraph());
			new_rotations.push_back({});
		}
		else
			vp = first_vertex_p;

		//find the other endpoints of the new cycle edges obtained by cutting f
		auto fp = f;
		auto fdp = f;

		std::cout << f << (g.signal(f)==1?'+':'-') << " " << u << std::endl;

		if(g.signal(f)==1){
			fp =  add_edge(up,vp,g.getGraph()).first;
			auto fp_index = ecount++;
			boost::put(edgei_map,fp,fp_index);
			g.edge_signals.push_back(g.signal(f));
			edges_by_index[fp_index] = fp;
			
		}else{
			fdp =  add_edge(u,vp,g.getGraph()).first;
			fp =  add_edge(up,v,g.getGraph()).first;

			auto fdp_index = boost::get(edgei_map,f);
			boost::put(edgei_map,fdp,fdp_index);
			edges_by_index[fdp_index] = fdp;

			auto fp_index = ecount++;
			boost::put(edgei_map,fp,fp_index);
			edges_by_index[fp_index] = fp;

			g.edge_signals.push_back(g.signal(f));

			to_remove.push_back(f);
			
		}

		std::cout << fp << " " << fdp << std::endl;

		//initialize rotations of vp and v
		//the other cycle edge will be added later
		if(g.signal(f)==1){
			new_rotations[v].push_back(fdp);
			new_rotations[vp].push_back(fp);
		}else{
			new_rotations[v].push_back(fp);
			new_rotations[vp].push_back(fdp);
		}

		//we now begin to build the rotations of u' and u
		//we assume that the other edge is already there
		new_rotations[up].push_back(fp);


		//find the indexes of e and f on u's rotation
		size_t e_index =0;
		size_t f_index =0;
		for(size_t i =0;i<g.rotations[u].size();++i){
			auto h = g.rotations[u][i];
			if(h==e)
				e_index=i;
			if(h==f)
				f_index=i;
		}

		std::cout << e_index << ' ' << f_index << ' ' << std::endl;

		for(size_t i =0;i<g.rotations[u].size();++i){
			auto h = g.rotations[u][i];
			auto h_index = boost::get(edgei_map,h);

			//h may be a left edge of some earlier vertex
			//in this case not_u will be the 
			auto a = edges_by_index[h_index];
			auto not_u = find_other_endpoint(a,u);

			std::cout << h << ' ' << a << ' ' << not_u << ' ';

			if(is_left(i,e_index,f_index)){
				std::cout << " <-left ";
				auto k =  add_edge(up,not_u,g.getGraph()).first;

				boost::put(edgei_map,k,h_index);
				new_rotations[up].push_back(k);
				edges_by_index[h_index] = k;
				to_remove.push_back(a);
			}else{
				if(h!=e && h!=f){
					std::cout << " <-added ";
					new_rotations[u].push_back(a);
				}
			}
		}
		std::cout << std::endl;

		new_rotations[u].push_back(fdp);
		
		//next step
		u = v;
		up = vp;
	}
	
	//this will update some edges
	//say, if an edge uv is a right edge of u and left of v
	//s.t. u < v in the cycle order, then the edge uv needs
	//to be updated to uv'
	for(auto&& pi_u : new_rotations){
		for(size_t i=0; i<pi_u.size(); i++){
			auto e = pi_u[i];
			auto e_index = boost::get(edgei_map,e);
			pi_u[i] = edges_by_index[e_index];
		}
	}

	for(auto&& e : to_remove){
		std::cout << "removed: " << e << std::endl;
		remove_edge(e,g.getGraph());
	}

	for(auto&& pi_u : new_rotations){
		for(auto&& e : pi_u){
			std::cout << (g.signal(e)==1?'+':'-') << e << ' ';
		}
		std::cout << std::endl;
	}


	//rotations of the vertices not in the cycle
	for(size_t i =0; i< g.rotations.size();i++){
		auto u = vertex(i,g.getGraph());
		auto& pi_u = g.rotations[i];

		//empty means u is not in the cycle
		if(new_rotations[i].empty()){
			for(size_t j =0; j< pi_u.size();j++){
				auto e = pi_u[j];
				auto e_index = boost::get(edgei_map,e);
				pi_u[j] = edges_by_index[e_index];
			}
			new_rotations[i] = std::move(pi_u);
		}
	}

	g.rotations = std::move(new_rotations);

	printGraph(g);
	printEmbedding(g);
	std::cout << "signals size: " << g.edge_signals.size() << std::endl;
}

inline auto eg2Genus(int euler_genus){
	return euler_genus/2;
}

inline auto eg2NOGenus(int euler_genus){
	return euler_genus;
}


///*
// * Cuts along a cycle in an embedded graph. The result is a new graph
// * where for each vertex v of the cycle is split into two vertices.
// * Their neighbors are determined by which "side" of the cycle they are.
// */
//template <typename Graph,EdgeRange<Graph> T>
//auto cutAlongCycle(EmbeddedGraph<Graph> g, const T& two_sided_cycle){
//	
//	std::vector<edge_t<Graph>> to_remove;
//	auto find_common = [&g](auto e,auto f){
//		auto [v,w] = gdraw::endpoints(g.getGraph(),e);
//		auto [x,y] = gdraw::endpoints(g.getGraph(),f);
//		if(v==x || v==y)
//			return v;
//		if(w==x || w==y)
//			return w;
//		return boost::graph_traits<Graph>::null_vertex();
//	};
//	auto get_next_edge = [&two_sided_cycle](auto ei){
//		if(ei+1 == two_sided_cycle.end())
//			return *(two_sided_cycle.begin());
//		return *(ei+1);
//	};
//	auto is_left = [](auto i,auto ei,auto fi){
//		if(ei< fi){
//			return i<ei || i>fi;
//		}
//			return fi<i && i<ei;
//	};
//	
//	auto edgei_map = get(boost::edge_index,g.getGraph());
//	auto ecount = num_edges(g.getGraph());
//
//	//TODO make new rotations
//	for(auto&& ei = two_sided_cycle.begin();ei!=two_sided_cycle.end();ei++){
//		auto e = *ei;
//		auto f = get_next_edge(ei);
//		auto u = find_common(e,f);
//		
//		std::cout << e << ' ' << f << ' ' << u << std::endl;
//		size_t e_index =0;
//		size_t f_index =0;
//		for(size_t i =0;i<g.rotations[u].size();++i){
//			auto h = g.rotations[u][i];
//			if(h==e)
//				e_index=i;
//			if(h==f)
//				f_index=i;
//		}
//
//		std::cout << e_index << ' ' << f_index << ' ' << std::endl;
//		auto ul = add_vertex(g.getGraph());
//
//		for(size_t i =0;i<g.rotations[u].size();++i){
//			auto h = g.rotations[u][i];
//			auto [v,w] = endpoints(g.getGraph(),h);
//			auto not_u = u!=v? v : w;
//
//			std::cout << h << ' ' << not_u << ' ' << std::endl;
//
//			if(is_left(i,e_index,f_index)){
//				auto k =  add_edge(ul,not_u,g.getGraph()).first;
//				boost::put(edgei_map,k,boost::get(edgei_map,h));
//				to_remove.push_back(h);
//			}
//			if(h==e || h==f){
//				auto k =  add_edge(ul,not_u,g.getGraph()).first;
//				boost::put(edgei_map,k,ecount++);
//			}
//		}
//	}
//
//	printGraph(g);
//
//	for(auto&& e : to_remove)
//		remove_edge(e,g.getGraph());
//
//	printGraph(g);
//}

int main(){
	GraphWrapper<AdjList> g {gdraw::getKpq<AdjList>(3,3)};
	GraphWrapper<AdjList> h {gdraw::getKn<AdjList>(6)};
	
	rotations_t<AdjList> rotations = {
		{edge(0,3,g.getGraph()).first,edge(0,4,g.getGraph()).first,edge(0,5,g.getGraph()).first},
		{edge(1,5,g.getGraph()).first,edge(1,4,g.getGraph()).first,edge(1,3,g.getGraph()).first},
		{edge(2,3,g.getGraph()).first,edge(2,5,g.getGraph()).first,edge(2,4,g.getGraph()).first},
		{edge(3,1,g.getGraph()).first,edge(3,2,g.getGraph()).first,edge(3,0,g.getGraph()).first},
		{edge(4,0,g.getGraph()).first,edge(4,2,g.getGraph()).first,edge(4,1,g.getGraph()).first},
		{edge(5,0,g.getGraph()).first,edge(5,2,g.getGraph()).first,edge(5,1,g.getGraph()).first}
	};

	auto edgei_map = get(boost::edge_index, g.getGraph());
	std::vector<int> esignals(num_edges(g.getGraph()),1);

	esignals[boost::get(edgei_map,edge(1,4,g.getGraph()).first)] = -1;
	esignals[boost::get(edgei_map,edge(2,5,g.getGraph()).first)] = -1;

	auto pg = ProjectivePlanarGraph<AdjList>(std::move(g),std::move(rotations),std::move(esignals));
	printGraph(pg);
	gdraw::printEmbedding(pg);

	std::vector<edge_t<AdjList>> cycle1 = {edge(0,3,pg.getGraph()).first,edge(3,1,pg.getGraph()).first,edge(1,4,pg.getGraph()).first,edge(4,0,pg.getGraph()).first};
	std::vector<edge_t<AdjList>> cycle2 = {edge(5,2,pg.getGraph()).first,edge(2,4,pg.getGraph()).first,edge(4,0,pg.getGraph()).first,edge(0,3,pg.getGraph()).first,edge(3,1,pg.getGraph()).first,edge(1,5,pg.getGraph()).first};
	//std::cout << std::boolalpha << gdraw::is1Sided(pg,cycle1) << std::endl;
	//std::cout << std::boolalpha << gdraw::is1Sided(pg,cycle2) << std::endl;

	cutAlongCycle(pg,cycle1);
	//cutAlongCycle(pg,cycle2);

}
