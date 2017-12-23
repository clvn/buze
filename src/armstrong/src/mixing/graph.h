#pragma once

#include <deque>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/depth_first_search.hpp>

using namespace boost;

namespace zzub {

struct metaplugin;
struct metaconnection;

// the graph type
typedef adjacency_list<vecS, vecS, bidirectionalS, int, int > plugin_map;

// various graph helper types
typedef graph_traits<plugin_map>::vertex_descriptor plugin_descriptor;
typedef graph_traits<plugin_map>::vertex_iterator plugin_iterator;
typedef graph_traits<plugin_map>::edge_descriptor connection_descriptor;
typedef graph_traits<plugin_map>::vertices_size_type size_type;
typedef graph_traits<plugin_map>::out_edge_iterator out_edge_iterator;
typedef graph_traits<plugin_map>::in_edge_iterator in_edge_iterator;

struct feedback_detector : public base_visitor<feedback_detector> {
	struct has_cycle { };
	typedef on_back_edge event_filter;
	std::vector<connection_descriptor>& back_edges;

	feedback_detector(std::vector<connection_descriptor>& result)
		:back_edges(result) { }
	template <class Vertex, class Graph>
	inline void operator()(Vertex u, Graph& g) {
		//cerr << u << " is a back edge!" << endl;
		back_edges.push_back(u);
	}
};

void topological_sort_kahn(plugin_map& tg, std::deque<plugin_descriptor>& input, std::vector<plugin_descriptor>& result);
void get_plugin_ids(std::vector<int>& ids, plugin_map& g, const std::vector<plugin_descriptor>& outputs);
void get_plugin_ptrs(std::vector<metaplugin*>& plugins, std::vector<metaplugin*>& ptrs, plugin_map& g, const std::vector<plugin_descriptor>& outputs);
void make_graph(std::vector<metaplugin*>& plugins, std::vector<metaconnection*>& connections, plugin_map& g);

};
