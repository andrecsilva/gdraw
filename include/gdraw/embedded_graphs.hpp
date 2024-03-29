#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <limits>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include <gdraw/graph_types.hpp>
#include <gdraw/util.hpp>
#include <gdraw/planar_graphs.hpp>

namespace gdraw{

/**
 * Checks whether the cycle is 1-sided (if the number of negative signal edges is odd).
 */
template <typename Graph,EdgeRange<Graph> T>
auto is1Sided(const EmbeddedGraph<Graph>& g, const T& cycle) -> bool{
	int signal=1;

	for(auto&& e : cycle){
		//std::cout << e <<std::endl;
		signal *= g.signal(e);
	}

	return signal == -1? true: false;
}

/**
 * Returns the smallest one-sided cycle. It is done using the fundamental
 * cycle method. See the book Graphs on Surface for a proof of this.
 */
template <typename Graph>
auto smallest1SidedCycle(const EmbeddedGraph<Graph>& g){
	
	using tree_type = decltype(bfsTree(std::declval<IndexedGraph<Graph>>(),
				std::declval<vertex_t<Graph>>()));
	decltype(fundamentalCycle(std::declval<IndexedGraph<Graph>>(),
				std::declval<tree_type>(),
				std::declval<edge_t<Graph>>())) cycle;

	size_t cycle_size = g.numVertices()+1;

	for(auto&& v : g.vertices()){
		auto v_bfs_tree = bfsTree(g,v);
		for(auto&& e : g.edges()){
			auto [a,b] = g.endpoints(e);
			if(e != v_bfs_tree[a] && e != v_bfs_tree[b]){
				//e is not a tree edge
				auto e_cycle = fundamentalCycle(g,v_bfs_tree,e);
				if(is1Sided(g,e_cycle) && e_cycle.size()<cycle_size){
					cycle = std::move(e_cycle);
					cycle_size = cycle.size();
				}
			}
		}
	}
	return cycle;
}


/*
 * Cuts along a non-contractible cycle in an embedded graph. The result is a new graph
 * where for each vertex v of the cycle is split into two vertices.
 * Their neighbors are determined by which "side" of the cycle they are.
 */
template <typename Graph,EdgeRange<Graph> T>
auto cutAlongCycle(EmbeddedGraph<Graph>& g, const T& nc_cycle){

	//maintain an edgelist by index
	auto edgei_map = get(boost::edge_index,g.getGraph());
	std::vector<edge_t<Graph>> edges_by_index(num_edges(g.getGraph()));
	auto ecount = num_edges(g.getGraph());

	for(auto e : range(edges(g.getGraph()))){
		auto ei = boost::get(edgei_map,e);
		edges_by_index[ei] = e;
	}

	auto add_edge_with_index = [&g,&edgei_map,&edges_by_index](auto&& u, auto&& v, auto&& ei){
			auto h = add_edge(u,v,g.getGraph()).first;
			boost::put(edgei_map,h,ei);
			edges_by_index[ei] = h;
			return h;
	};

	auto add_extra_edge = [&g,&edgei_map,&edges_by_index,&ecount](auto&& u, auto&& v){
			auto h = add_edge(u,v,g.getGraph()).first;
			boost::put(edgei_map,h,ecount++);
			edges_by_index.push_back(h);
			return h;
	};

	//we need to buffer all the edge that will be removed to avoid
	//invalid pointers midway
	std::vector<edge_t<Graph>> to_remove;

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

	//grab the next edge in a nc_cycle
	auto get_next_edge = [&nc_cycle](auto ei){
		if(ei+1 == nc_cycle.end())
			return *(nc_cycle.begin());
		return *(ei+1);
	};

	//checks whether an edge is on the left (right) side of the cycle
	auto is_left = [](auto i,auto ei,auto fi){
		if(ei< fi){
			return i<ei || i>fi;
		}
			return fi<i && i<ei;
	};

	//will contain the new rotations 
	rotations_t<Graph> new_rotations(num_vertices(g.getGraph()));

	auto e = *(nc_cycle.begin());
	auto f = get_next_edge(nc_cycle.begin());
	auto u = find_common(e,f);
	auto up = add_vertex(g.getGraph());
	new_rotations.push_back({});


	auto first_vertex = u;
	auto first_vertex_p = up;

	//finds the other endpoint of e that is not u
	auto find_other_endpoint = [&g](auto e,auto u){
		auto [a,b] = gdraw::endpoints(g.getGraph(),e);
		return  a!=u? a : b;
	};

	// -e- u -f- v
	// up = u' is the left "side" of u, similarly for v
	// fp = f', fdp = f'', left and right side of f w.r.t. u
	for(auto&& ei = nc_cycle.begin();ei!=nc_cycle.end();ei++){
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

		//find the other endpoints of the new nc_cycle edges obtained by cutting f
		auto fp = f;
		auto fdp = f;

		//std::cout << f << (g.signal(f)==1?'+':'-') << " " << u << std::endl;

		if(g.signal(f)==1){
			fp = add_extra_edge(up,vp);
			g.edge_signals.push_back(g.signal(f));
			
		}else{
			fdp =  add_edge_with_index(u,vp,boost::get(edgei_map,f));
			fp =  add_extra_edge(up,v);
			g.edge_signals.push_back(g.signal(f));

			to_remove.push_back(f);
		}

		//std::cout << fp << " " << fdp << std::endl;

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

		//std::cout << e_index << ' ' << f_index << ' ' << std::endl;

		for(size_t i =0;i<g.rotations[u].size();++i){
			auto h = g.rotations[u][i];
			auto h_index = boost::get(edgei_map,h);

			//h may be a left edge of some earlier vertex
			//in this case not_u will be the 
			auto a = edges_by_index[h_index];
			auto not_u = find_other_endpoint(a,u);

			//std::cout << h << ' ' << a << ' ' << not_u << ' ';

			if(is_left(i,e_index,f_index)){
				//std::cout << " <-left ";
				auto k = add_edge_with_index(up,not_u,h_index);
				new_rotations[up].push_back(k);
				to_remove.push_back(a);
			}else{
				if(h!=e && h!=f){
					//std::cout << " <-added ";
					new_rotations[u].push_back(a);
				}
			}
		}
		//std::cout << std::endl;

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

	//rotations of the vertices not in the cycle
	for(size_t i =0; i< g.rotations.size();i++){
		//auto u = vertex(i,g.getGraph());
		auto& pi_u = g.rotations[i];

		//empty means u is not in the cycle
		if(new_rotations[i].empty()){
			for(size_t j =0; j< pi_u.size();j++){
				auto e = pi_u[j];
				auto e_index = boost::get(edgei_map,e);
				//std::cout << e_index << ':' << edges_by_index[e_index] << std::endl;
				pi_u[j] = edges_by_index[e_index];
			}
			new_rotations[i] = std::move(pi_u);
		}
	}

	for(auto&& e : to_remove){
		//std::cout << "removed: " << e << std::endl;
		remove_edge(e,g.getGraph());
	}


	g.rotations = std::move(new_rotations);

	//printGraph(g);
	//printEmbedding(g);
	//std::cout << "signals size: " << g.edge_signals.size() << std::endl;
	return g;
}

/**
 * Changes the rotations of g until all the edges of the tree
 * are positive.
 */
template <typename Graph>
auto positiveSpanningTree(EmbeddedGraph<Graph>& g, std::vector<std::optional<edge_t<Graph>>>& spanning_tree, vertex_t<Graph>& root){

	//auto root = *(vertices(g.getGraph()));
	//auto parent = dfsForest(g,root);

	auto find_other_endpoint = [&g](auto e,auto u){
		auto [a,b] = gdraw::endpoints(g.getGraph(),e);
		return  a!=u? a : b;
	};

	std::list<vertex_t<Graph>> queue;
	auto edgei_map = get(boost::edge_index, g.getGraph());

	queue.push_back(root);

	while(!queue.empty()){
		auto u = queue.front();
		queue.pop_front();

		for(auto&& e : range(out_edges(u,g.getGraph()))){
			auto v = find_other_endpoint(e,u);
			if(spanning_tree[v] == e){
				queue.push_back(v);
				if(g.signal(e) == -1){
					std::reverse(g.rotations[v].begin(),g.rotations[v].end());
					for(auto&& f : range(out_edges(v,g.getGraph()))){
							auto f_index = boost::get(edgei_map,f);
							g.edge_signals[f_index] *=-1;
					}
				}
			}
		}
	}
}

/**
 * Returns true if the g is embedded in a orientable surface.
 * Modifies the graph as in positiveSpanningTree.
 */
template <typename Graph>
auto isOrientable(EmbeddedGraph<Graph>& g) -> bool{
	auto root = *(vertices(g.getGraph()).first);
	auto spanning_tree = dfsForest(g,root);

	positiveSpanningTree(g,spanning_tree,root);

	for(auto&& e : range(edges(g.getGraph()))){
		if(g.signal(e) == -1)
			return false;
	}

	return true;
}

/**
 * Returns the facial walk of the face incident with  the u->v side of the edge e.
 */
template <typename Graph>
auto facialWalk(const EmbeddedGraph<Graph>& g,
	       	const edge_t<Graph>& e,
	       	const vertex_t<Graph>& u,
		const bool starting_signal,
	       	const std::vector<std::vector<edge_t<Graph>>>& next_edge,
	       	const std::vector<std::vector<edge_t<Graph>>>& prev_edge,
	       	std::vector<std::vector<bool>>& visited
		){

	//std::cout << e << ' ' << u << (starting_signal?'+':'-') << std::endl;

	auto next_in_rotation = [&g,&next_edge,&prev_edge](auto u,auto e,auto positive_signal){
		auto [a,b] = g.endpoints(e);
		auto v = a!=u ? a : b;

		if(positive_signal){
			if(g.index(u) <= g.index(v))
				return next_edge[g.index(e)][0];
			return next_edge[g.index(e)][1];
		}
		else{
			if(g.index(u) <= g.index(v))
				return prev_edge[g.index(e)][0];
			return prev_edge[g.index(e)][1];
		}

	};

	//positive edge | negative edge
	//0: (u,v) u + = (u,v) v - | (u,v) u + = (u,v) v + 
	//1: (u,v) u - = (u,v) v + | (u,v) u - = (u,v) v -
	auto mark_visited = [&g,&visited](auto u,auto e,auto positive_signal){
		//std::cout << "m: " << e << ' ' << u << ' ' << (positive_signal?'+':'-') << std::endl;
		auto [a,b] = g.endpoints(e);
		auto v = a!=u ? a : b;

		if(positive_signal){
			if(g.index(u) <= g.index(v))
				visited[g.index(e)][0] = true;
			else{
				if(g.signal(e)==1)
					visited[g.index(e)][1] = true;
				else
					visited[g.index(e)][0] = true;
			}
		}
		else{
			if(g.index(u) <= g.index(v))
				visited[g.index(e)][1] = true;
			else{
				if(g.signal(e)==1)
					visited[g.index(e)][0] = true;
				else
					visited[g.index(e)][1] = true;
			}
		}
	};

	std::vector<edge_t<Graph>> facial_walk;
	bool positive_signal = starting_signal;

	auto f = e;
	auto v = u; do{
		facial_walk.push_back(f);
		//std::cout << f << ' ' <<  v << ' ' << std::boolalpha << (positive_signal?'+':'-') << std::endl;
		mark_visited(v,f,positive_signal);

		if(g.signal(f)==-1)
			positive_signal = !positive_signal;

		auto [a,b] = g.endpoints(f);
		v = a!=v ? a : b;
		f = next_in_rotation(v,f,positive_signal);
	}while(f!=e || v!=u || positive_signal!=starting_signal);
	//std::cout << f << ' ' <<  v << std::endl;
	
//	for(auto&& e : g.edges()){
//		std::cout << e << ' '  << visited[g.index(e)][0] << ' ' << visited[g.index(e)][1] << std::endl;
//	}

	return facial_walk;
}


/**
 * Returns all the possible facial walks for an embedded graph g.
 */
template <typename Graph>
auto allFacialWalks(const EmbeddedGraph<Graph>& g){
	
	//These are the pi_v(e) and its inverse.
	std::vector<std::vector<edge_t<Graph>>> next_edge(g.numEdges(),std::vector<edge_t<Graph>>(2));
	std::vector<std::vector<edge_t<Graph>>> prev_edge(g.numEdges(),std::vector<edge_t<Graph>>(2));

	for(auto&& u : g.vertices()){
		//std::cout << u << std::endl;
		for(size_t i=0; i<g.rotations[g.index(u)].size();i++){
			auto e = g.rotations[g.index(u)][i];
			auto next = g.rotations[g.index(u)][(i+1)%g.rotations[g.index(u)].size()];
			auto prev = g.rotations[g.index(u)][i==0?g.rotations[g.index(u)].size()-1 : (i-1)];
			//std::cout << e << ' ' << next << ' ' << prev << std::endl;;

			auto [a,b] = g.endpoints(e);
			auto v = a!=u ? a : b;

			if(g.index(u) <= g.index(v))
				next_edge[g.index(e)][0] = next;
			else
				next_edge[g.index(e)][1] = next;

			if(g.index(u) <= g.index(v))
				prev_edge[g.index(e)][0] = prev;
			else
				prev_edge[g.index(e)][1] = prev;
		}
	}

	//printGraph(g);
	//printEmbedding(g);
	//for(auto&& e : g.edges()){
	//	std::cout << e << std::endl;
	//	std::cout << prev_edge[g.index(e)][0] << ' ' << prev_edge[g.index(e)][1] << std::endl;
	//	std::cout << next_edge[g.index(e)][0] << ' ' << next_edge[g.index(e)][1] << std::endl;
	//}

	std::vector<std::vector<bool>> visited(g.numEdges(),std::vector<bool>(2,false));
	std::vector<std::vector<edge_t<Graph>>> facial_walks;

	for(auto&& e : g.edges()){
		auto [u,v] = g.endpoints(e);
		if(g.index(v) < g.index(u)){
			auto temp = v;
			v = u;
			u = temp;
		}

		if(!visited[g.index(e)][0])
			facial_walks.push_back(facialWalk(g,e,u,true,next_edge,prev_edge,visited));
		if(!visited[g.index(e)][1])
			facial_walks.push_back(facialWalk(g,e,u,false,next_edge,prev_edge,visited));
	}

	return facial_walks;

}

/**
 * Returns the Euler genus of the graph.
 */
template <typename Graph>
auto eulerGenus(EmbeddedGraph<Graph>& g) -> int{
	return 0;
}


}//namespace
