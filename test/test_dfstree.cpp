#include <iostream>
#include <cassert>

#include <boost/graph/adjacency_list.hpp>

#include <gdraw/graph_types.hpp>

#include <gdraw/generators.hpp>
#include <gdraw/util.hpp>

#define ASSERT(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

using AdjList = boost::adjacency_list<
	boost::vecS
	,boost::vecS
	,boost::undirectedS
	,boost::property<boost::vertex_index_t,size_t>
	,boost::property<boost::edge_index_t,size_t>
	>; 

using namespace gdraw;

auto test_dfsTree(){
	IndexedGraph<AdjList> g = getKpq<AdjList>(4,4);

	auto u = vertex(2,g.getGraph());

	auto forest = dfsForest(g,u);

	std::vector<std::optional<edge_t<AdjList>>> solution = {
		edge(0,4,g.getGraph()).first,
		edge(1,5,g.getGraph()).first,
		{},
		edge(3,6,g.getGraph()).first,
		edge(4,2,g.getGraph()).first,
		edge(5,0,g.getGraph()).first,
		edge(6,1,g.getGraph()).first,
		edge(7,3,g.getGraph()).first
	};
	for(size_t i=0; i<forest.size();i++){
		ASSERT(forest[i] == solution[i]);
	}
}

auto test_dfsForest(){
	IndexedGraph<AdjList> g(8);
	add_edge(0,1,g.getGraph());
	add_edge(0,2,g.getGraph());
	add_edge(1,3,g.getGraph());
	add_edge(1,4,g.getGraph());
	add_edge(5,6,g.getGraph());
	add_edge(6,7,g.getGraph());

	auto u = vertex(0,g.getGraph());
	auto forest = dfsForest(g,u);
	std::vector<std::optional<edge_t<AdjList>>> solution = {
		{},
		edge(1,0,g.getGraph()).first,
		edge(2,0,g.getGraph()).first,
		edge(3,1,g.getGraph()).first,
		edge(4,1,g.getGraph()).first,
		{},
		edge(6,5,g.getGraph()).first,
		edge(7,6,g.getGraph()).first
	};
	for(size_t i=0; i<forest.size();i++){
		ASSERT(forest[i] == solution[i]);
	}
}

auto test_bfstree(){
	auto g = IndexedGraph{AdjList(7)};

	g.addEdge(0,1);
	g.addEdge(0,2);

	g.addEdge(1,3);
	g.addEdge(1,4);

	g.addEdge(2,5);
	g.addEdge(2,6);

	g.addEdge(3,5);
	g.addEdge(4,2);

	auto bfs_tree = bfsTree(g,0);

	size_t count = 0;
	for(auto&& e : bfs_tree){
		if(e){
			count++;
			//std::cout << e.value() << ' ';
		}
		else{
			//std::cout << "none" << ' ';
		}
	}
	//std::cout << std::endl;
	
	ASSERT(count == 6);

	//for(auto&& e : fedges)
	//	std::cout << e << ' ';
	//std::cout << std::endl;
}


int main(){
	std::cout << "Testing : " << __FILE__ << std::endl;

	test_dfsTree();
	test_dfsForest();
	test_bfstree();
}
