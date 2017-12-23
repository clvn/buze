#include <vector>
#include <deque>
#include <map>
#include "graph.h"
#include "mixer.h"

using std::map;
using std::deque;
using std::vector;

namespace zzub {

// http://en.wikipedia.org/wiki/Topological_sorting
void topological_sort_kahn(plugin_map& tg, std::deque<plugin_descriptor>& input, std::vector<plugin_descriptor>& result) {
	while (input.size()) {
		plugin_descriptor n = input.front();
		input.pop_front();
		result.push_back(n);
		graph_traits<plugin_map>::out_edge_iterator out, out_end;
		boost::tie(out, out_end) = out_edges(n, tg);
		std::deque<plugin_descriptor> adj;
		for (graph_traits<plugin_map>::out_edge_iterator i = out; i != out_end; ++i) {
			adj.push_back(target(*i, tg));
		}
		for (std::deque<plugin_descriptor>::iterator i = adj.begin(); i != adj.end(); ++i) {
			remove_edge(n, *i, tg);
			graph_traits<plugin_map>::in_edge_iterator in, in_end;
			boost::tie(in, in_end) = in_edges(*i, tg);
			if ((in_end - in) == 0) {
				input.push_back(*i);
			}
		}
	}
}

void get_plugin_ids(std::vector<int>& ids, plugin_map& g, const std::vector<plugin_descriptor>& outputs) {
	for (std::vector<plugin_descriptor>::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
		ids.push_back(g[*i]);
	}
}

void get_plugin_ptrs(std::vector<shared_ptr<metaplugin> >& plugins, std::vector<metaplugin*>& ptrs, plugin_map& g, const std::vector<plugin_descriptor>& outputs) {
	for (std::vector<plugin_descriptor>::const_iterator i = outputs.begin(); i != outputs.end(); ++i) {
		int plugin_id = g[*i];
		ptrs.push_back(plugins[plugin_id].get());
	}
}

// make_graph(...) is not a member of the mixer class to keep the boost stuff
// local to mixer.cpp

void make_graph(std::vector<boost::shared_ptr<metaplugin> >& plugins, std::vector<boost::shared_ptr<metaconnection> >& connections, plugin_map& g) {
    
	map<int, plugin_descriptor> id2desc;

	//vector<metaplugin*>& plugins = mix.plugins.get_next();
	//vector<metaconnection*>& connections = mix.connections.get_next();

	for (vector<shared_ptr<metaplugin> >::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if (!*i || ((*i)->info->flags & zzub_plugin_flag_is_connection) != 0) continue;
		plugin_descriptor desc = add_vertex(g);
		g[desc] = (*i)->id;
		id2desc[(*i)->id] = desc;
	}

	for (vector<shared_ptr<metaconnection> >::iterator i = connections.begin(); i != connections.end(); ++i) {
		if (!*i) continue;

		plugin_descriptor to_plugin = id2desc[(*i)->to_plugin_id];
		plugin_descriptor from_plugin = id2desc[(*i)->from_plugin_id];
		assert(to_plugin != graph_traits<plugin_map>::null_vertex());
		assert(from_plugin != graph_traits<plugin_map>::null_vertex());

		std::pair<connection_descriptor, bool> edge_instance_p = add_edge(to_plugin, from_plugin, g);
		g[edge_instance_p.first] = (*i)->id;
	}

}

//
// song order
//

void plugingraph::make(std::vector<shared_ptr<metaplugin> >& plugins, std::vector<boost::shared_ptr<metaconnection> >& connections) {
	vector<metaplugin*>& result = workorder;//.get_next();
	//workorder.is_dirty = true;

	result.clear();

	// build graph
	plugin_map tg;
	make_graph(plugins, connections, tg);

	// find back edges
	vector<connection_descriptor> backedgedescs;
	depth_first_search(tg, visitor(make_dfs_visitor(feedback_detector(backedgedescs))));

	// remove back edges from temp graph so we can perform a topological sort
	// (TODO: test with multiple loops - are connection_descriptors stable after removing some other edge??)
	for (vector<connection_descriptor>::iterator i = backedgedescs.begin(); i != backedgedescs.end(); ++i) {
		backedges.push_back(connections[tg[*i]].get());
		remove_edge(*i, tg);
	}

	// find roots in the graph, e.g master, and machines w/o outputs
	// TODO: should scan the roots and find the one containing the master and put it last - fixes auxbus
	vector<plugin_descriptor> roots;
	plugin_iterator v, v_end;
	for (tie(v, v_end) = vertices(tg); v != v_end; ++v) {
		zzub::in_edge_iterator e, e_end;
		boost::tie(e, e_end) = in_edges(*v, tg);
		if ((e_end - e) == 0) roots.push_back(*v);
	}

	// do a topological sort from each root 
	// TODO: ensure plugins related to the master are put last in the final work order
	for (vector<plugin_descriptor>::iterator i = roots.begin(); i != roots.end(); ++i) {
		vector<metaplugin*> ids;
		vector<plugin_descriptor> outputs;
		deque<plugin_descriptor> inputs;
		inputs.push_back(*i);

		int root_id = tg[*i];
		metaplugin* m = plugins[root_id].get();
		assert(m != 0);

		// find plugins that send sound into this root, in topological order.
		// topological_sort_kahn will remove edges from the temp graph.
		topological_sort_kahn(tg, inputs, outputs);

		get_plugin_ptrs(plugins, ids, tg, outputs);

		result.insert(result.begin(), ids.rbegin(), ids.rend());
	}
/*
	cerr << "-------------------------------------------------------" << endl;
	cerr << "Remaining out-edges:" << endl;
	out_edge_iterator out, out_end;
	for (tie(v, v_end) = vertices(tg); v != v_end; ++v) {
		for (tie(out, out_end) = out_edges(*v, tg); out != out_end; ++out) {
			cerr << source(*out, tg) << " -> " << target(*out, tg) << endl;
		}
	}

	cerr << "Remaining in-edges:" << endl;
	in_edge_iterator in, in_end;
	for (tie(v, v_end) = vertices(tg); v != v_end; ++v) {
		for (tie(in, in_end) = in_edges(*v, tg); in != in_end; ++in) {
			cerr << source(*in, tg) << " -> " << target(*in, tg) << endl;
		}
	}

	cerr << "-------------------------------------------------------" << endl;
	for (std::vector<plugin_descriptor>::iterator i = result.begin(); i != work_order.end(); ++i) {
		cerr << *i << ", ";
	}
	cerr << endl;*/
}

}
