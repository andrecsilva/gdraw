#pragma once 

#include <iostream>
#include <variant>

#include <gdraw/graph_types.hpp>
#include <gdraw/util.hpp>
#include <gdraw/planar_graphs.hpp>

/**
 * Removes isolated vertices (i.e. degree 0) from the graph.
 */

namespace gdraw{

inline auto disjointEdges(auto&& g,auto&& e,auto&& f){
	auto [u,v] = endpoints(g.getGraph(),e);
	auto [a,b] = endpoints(g.getGraph(),f);
	return (u != a && u != b && v!=a && v!=b);
}

template <typename Graph>
auto planarXNumber(GraphWrapper<Graph> g, size_t k) -> std::optional<PlanarGraph<Graph>>{
	auto ecount = num_edges(g.getGraph());
	auto vcount = num_vertices(g.getGraph());
	std::vector<edge_t<Graph>> edges_by_index(ecount+2*k);
	auto edgei_map = get( boost::edge_index, g.getGraph());

	for([[maybe_unused]]auto&& _ : std::views::iota((size_t)0,k)){
		add_vertex(g.getGraph());
	}

	for(auto&& e : range(edges(g.getGraph()))){
		auto ei = get(edgei_map,e);
		edges_by_index[ei] = e;
	}

	auto add_edge_with_index = [&g,&edgei_map,&edges_by_index,&ecount](auto&& u, auto&& v, auto&& ei){
			auto h = add_edge(u,v,g.getGraph()).first;
			boost::put(edgei_map,h,ei);
			edges_by_index[ei] = h;
			return h;
	};

	auto fake_cross = [&g,&vcount,&add_edge_with_index,&ecount](auto&& e,auto&& f,auto&& ei, auto&& fi){
		//std::cout << e << 'x' << f << std::endl;
		//printGraph(g);
		auto [u,v] = endpoints(g.getGraph(),e);
		auto [a,b] = endpoints(g.getGraph(),f);

		//auto e2 = edge(u,v,g.getGraph()).first;
		//auto f2 = edge(a,b,g.getGraph()).first;

		auto w = vertex(vcount,g.getGraph());

		remove_edge(e,g.getGraph());
		remove_edge(f,g.getGraph());

		add_edge_with_index(w,u,ei);
		add_edge_with_index(w,a,fi);
		add_edge_with_index(w,v,ecount++);
		add_edge_with_index(w,b,ecount++);


		vcount++;

	};

	auto uncross = [&g,&vcount,&ecount,&add_edge_with_index](auto&& e, auto&& f,auto&& ei,auto&& fi){
		//std::cout << "-" << e << 'x' << f << std::endl;
		vcount--;
		clear_vertex(vertex(vcount,g.getGraph()),g.getGraph());
		ecount-=2;

		auto [u,v] = endpoints(g.getGraph(),e);
		auto [a,b] = endpoints(g.getGraph(),f);

		//revalidate descriptors...
		e = add_edge_with_index(u,v,ei);
		f = add_edge_with_index(a,b,fi);
	};

	//Translates the Variant result into an optional
	auto planar_test = [](auto&& g){
		auto v = gdraw::planeEmbedding(std::move(g));
		std::optional<PlanarGraph<Graph>> pg;
		if(std::holds_alternative<PlanarGraph<Graph>>(v))
			pg = std::move(std::get<0>(v));
		else//Move the graph back
			g = std::move(std::get<1>(v));
		return pg;
	};

	auto result = planarXNumberRecursion(g,k,edges_by_index,planar_test,fake_cross,uncross);
	if(result)
		removeIsolatedVertices(result.value().getGraph());
	return result;
}

template <typename Graph>
auto planarXNumberRecursion(GraphWrapper<Graph>& g, size_t k, std::vector<edge_t<Graph>>& edges_by_index, auto& planar_test, auto& fake_cross, auto& uncross){
	//std::cout << "k = "  << k<< std::endl;
	if(k<=1){
		auto variant_result = gdraw::planeEmbedding(std::move(g));
		if(std::holds_alternative<PlanarGraph<Graph>>(variant_result))//xnumber = 0?
			return std::optional<PlanarGraph<Graph>>(std::move(std::get<0>(variant_result)));
		//We now use the Edges of the Kuratowski Subgraph instead of every pair of edges
		auto& neg = std::get<1>(variant_result);
		auto kuratowski_subgraph = std::move(neg.forbidden_subgraph);
		g = std::move(neg);
		
		auto edgei_map = boost::get(boost::edge_index,g.getGraph());

		for(size_t i=0; i < kuratowski_subgraph.size(); i++){
			auto& e = kuratowski_subgraph[i];
			auto ei = boost::get(edgei_map,e);
			for(size_t j=i; j < kuratowski_subgraph.size(); j++){
				auto& f = kuratowski_subgraph[j];
				auto fi = boost::get(edgei_map,f);
				if(disjointEdges(g,e,f)){
					fake_cross(e,f,ei,fi);
					auto variant_result = gdraw::planeEmbedding(std::move(g));
					if(std::holds_alternative<PlanarGraph<Graph>>(variant_result))//xnumber = 0?
						return std::optional<PlanarGraph<Graph>>(std::move(std::get<0>(variant_result)));
					else
						g = std::move(std::get<1>(variant_result));
					uncross(e,f,ei,fi);
				}
			}
		}
		return std::optional<PlanarGraph<Graph>>();
	}
	else{
		auto result = planarXNumberRecursion(g,k-1,edges_by_index,planar_test,fake_cross,uncross);
		if(result)
			return result;
		auto ecount = num_edges(g.getGraph());
		for(size_t i =0 ; i< ecount ; i++){
			auto e = edges_by_index[i];
			for(size_t j =i; j< ecount ; j++){
				auto f = edges_by_index[j];
				if(disjointEdges(g,e,f)){
					fake_cross(e,f,i,j);
					auto result = planarXNumberRecursion(g,k-1,edges_by_index,planar_test,fake_cross,uncross);
					if(result)
						return result;
					uncross(e,f,i,j);
				}
			}
		}
	}
	return std::optional<PlanarGraph<Graph>>();

}


template <typename Graph, typename Function>
auto xNumberRecursion(GraphWrapper<Graph>& g,size_t k, Function& embedd_test, std::vector<edge_t<Graph>>& edges_by_index, auto& fake_cross, auto& uncross){
	//std::cout << "k = "  << k<< std::endl;
	if(k<1){
		return embedd_test(g);
	}
	else{
		auto result = xNumberRecursion(g,k-1,embedd_test,edges_by_index,fake_cross,uncross);
		if(result)
			return result;
		auto ecount = num_edges(g.getGraph());
		for(size_t i =0 ; i< ecount ; i++){
			auto e = edges_by_index[i];
			for(size_t j =i; j< ecount ; j++){
				auto f = edges_by_index[j];
				if(disjointEdges(g,e,f)){
					fake_cross(e,f,i,j);
					auto result = xNumberRecursion(g,k-1,embedd_test,edges_by_index,fake_cross,uncross);
					if(result)
						return result;
					uncross(e,f,i,j);
				}
			}
		}
	}
	return decltype(embedd_test(g))();
}

//TODO: would prefer something more strongly typed 
template <typename Graph, typename Function>
auto xNumber(GraphWrapper<Graph> g, size_t k, Function embedd_test){
	auto ecount = num_edges(g.getGraph());
	auto vcount = num_vertices(g.getGraph());
	std::vector<edge_t<Graph>> edges_by_index(ecount+2*k);
	auto edgei_map = get( boost::edge_index, g.getGraph());

	for([[maybe_unused]]auto&& _ : std::views::iota((size_t)0,k)){
		add_vertex(g.getGraph());
	}

	for(auto&& e : range(edges(g.getGraph()))){
		auto ei = get(edgei_map,e);
		edges_by_index[ei] = e;
	}

	auto add_edge_with_index = [&g,&edgei_map,&edges_by_index,&ecount](auto&& u, auto&& v, auto&& ei){
			auto h = add_edge(u,v,g.getGraph()).first;
			boost::put(edgei_map,h,ei);
			edges_by_index[ei] = h;
			return h;
	};

	auto fake_cross = [&g,&vcount,&add_edge_with_index,&ecount](auto&& e,auto&& f,auto&& ei, auto&& fi){
		//std::cout << e << 'x' << f << std::endl;
		//printGraph(g);
		auto [u,v] = endpoints(g.getGraph(),e);
		auto [a,b] = endpoints(g.getGraph(),f);

		//auto e2 = edge(u,v,g.getGraph()).first;
		//auto f2 = edge(a,b,g.getGraph()).first;

		auto w = vertex(vcount,g.getGraph());

		remove_edge(e,g.getGraph());
		remove_edge(f,g.getGraph());

		add_edge_with_index(w,u,ei);
		add_edge_with_index(w,a,fi);
		add_edge_with_index(w,v,ecount++);
		add_edge_with_index(w,b,ecount++);


		vcount++;

	};

	auto uncross = [&g,&vcount,&ecount,&add_edge_with_index](auto&& e, auto&& f,auto&& ei,auto&& fi){
		//std::cout << "-" << e << 'x' << f << std::endl;
		vcount--;
		clear_vertex(vertex(vcount,g.getGraph()),g.getGraph());
		ecount-=2;

		auto [u,v] = endpoints(g.getGraph(),e);
		auto [a,b] = endpoints(g.getGraph(),f);

		//revalidate descriptors...
		e = add_edge_with_index(u,v,ei);
		f = add_edge_with_index(a,b,fi);
	};

	auto result = xNumberRecursion(g,k,embedd_test,edges_by_index,fake_cross,uncross);
	if(result)
		removeIsolatedVertices(result.value().getGraph());
	return result;
}

} //namespace
