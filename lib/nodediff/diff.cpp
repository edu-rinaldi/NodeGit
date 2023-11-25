#include "diff.h"

#include "script.h"
#include "utility/utility.h"

#include <execution>

///
/// Operators
///
namespace nd
{
bool operator==(const node_diff& node_diff1, const node_diff& node_diff2)
{
	return node_diff1.node_values == node_diff2.node_values &&
		   node_diff1.node_references == node_diff2.node_references &&
		   node_diff1.graph_references == node_diff2.graph_references &&
		   node_diff1.texture_references == node_diff2.texture_references &&
		   node_diff1.input_references == node_diff2.input_references;
}

bool operator==(const node_change& node_change1, const node_change& node_change2)
{
	return node_change1.op == node_change2.op && node_change1.diff == node_change2.diff;
}
}; // namespace nd

///
/// Reference renaming functions
///
namespace nd
{
/*
 * Renames references stored by a node with the matched ones. This function is used for add operation,
 * because it can happen that a new node is added and it could contains references to nodes already existing in the
 * ancestor.
 *
 * Example:
 * ancestor_graph = {"node1": nd::node{..}, "node2": nd::node{..}};
 * version_graph = {"nodeA": nd::node{..}, "nodeB": nd::node{..}, "nodeC": nd::node{..}};
 * nodeC refers to nodeB, which, for example, is matched with node2.
 * When adding nodeC to ancestor_graph, its reference to nodeB should be changed in node2.
 */
static void rename_node(node& node, const ref_match<node_ref>& node_matches, const ref_match<graph_ref>& graph_matches)
{
	for (auto& [property_name, version_ref] : node.node_references)
	{
		// If the referenced node has a match in ancestor ==> rename
		if (node_matches.has_match_in_ancestor(version_ref)) { version_ref = node_matches.to_ancestor(version_ref); }
	}
	for (auto& [property_name, version_ref] : node.graph_references)
	{
		// If the referenced graph has a match in ancestor ==> rename
		if (graph_matches.has_match_in_ancestor(version_ref)) { version_ref = graph_matches.to_ancestor(version_ref); }
	}
	for (auto& [socket_name, version_edge] : node.input_references)
	{
		// If the referenced node has a match in ancestor ==> rename
		if (node_matches.has_match_in_ancestor(version_edge.node))
		{
			version_edge.node = node_matches.to_ancestor(version_edge.node);
		}
	}
}

/*
 * Renames references stored by all nodes in a graph with the matched ones.
 */
static void rename_graph(graph& graph, const ref_match<graph_ref>& graph_matches)
{
	for (auto& [node_id, node] : graph.nodes)
	{
		rename_node(node, {}, graph_matches);
	}
}
}; // namespace nd

///
/// Functions for diffing node's properties
///
namespace nd
{
int diff_node_values(const property_map<value>& ancestor_values, const property_map<value>& version_values,
					 property_map<value>* diff)
{
	int count = 0;
	for (const auto& [property_name, version_value] : version_values)
	{
		// If they have different values ==> diff
		if (ancestor_values.at(property_name) != version_value)
		{
			++count;
			if (diff) { (*diff)[property_name] = version_value; }
		}
	}
	return count;
}

int diff_node_values(const node& ancestor_node, const node& version_node, property_map<value>* diff)
{
	return diff_node_values(ancestor_node.node_values, version_node.node_values, diff);
}

int diff_node_references(const property_map<node_ref>& ancestor_node_refs,
						 const property_map<node_ref>& version_node_refs, const ref_match<node_ref>& node_matches,
						 property_map<node_ref>* diff)
{
	int count = 0;
	for (const auto& [property_name, version_ref] : version_node_refs)
	{
		const node_ref& ancestor_ref = ancestor_node_refs.at(property_name);

		// Check if the referenced node has no match with ancestor (i.e. is added in version)
		// If so ==> no rename (just use the version id)
		if (!node_matches.has_match_in_ancestor(version_ref))
		{
			++count;
			if (diff) { (*diff)[property_name] = version_ref; }
		}
		else
		{
			// If they have different references ==> diff
			const node_ref& matched_version_ref = node_matches.to_ancestor(version_ref);
			if (matched_version_ref != ancestor_ref)
			{
				++count;
				if (diff) { (*diff)[property_name] = matched_version_ref; }
			}
		}
	}
	return count;
}

int diff_node_references(const node& ancestor_node, const node& version_node, const ref_match<node_ref>& node_matches,
						 property_map<node_ref>* diff)
{
	return diff_node_references(ancestor_node.node_references, version_node.node_references, node_matches, diff);
}

int diff_graph_references(const property_map<graph_ref>& ancestor_graph_refs,
						  const property_map<graph_ref>& version_graph_refs, const ref_match<graph_ref>& graph_matches,
						  property_map<graph_ref>* diff)
{
	int count = 0;
	for (const auto& [property_name, version_ref] : version_graph_refs)
	{
		const graph_ref& ancestor_ref = ancestor_graph_refs.at(property_name);

		// Check if the referenced graph has no match with ancestor (i.e. is added in version)
		// If so ==> no rename (just use the version id)
		if (!graph_matches.has_match_in_ancestor(version_ref))
		{
			++count;
			if (diff) { (*diff)[property_name] = version_ref; }
		}
		else
		{
			// If they have different references ==> diff
			const graph_ref& matched_version_ref = graph_matches.to_ancestor(version_ref);
			if (matched_version_ref != ancestor_ref)
			{
				++count;
				if (diff) { (*diff)[property_name] = matched_version_ref; }
			}
		}
	}
	return count;
}
int diff_graph_references(const node& ancestor_node, const node& version_node,
						  const ref_match<graph_ref>& graph_matches, property_map<graph_ref>* diff)
{
	return diff_graph_references(ancestor_node.graph_references, version_node.graph_references, graph_matches, diff);
}

int diff_texture_references(const property_map<texture_ref>& ancestor_texture_refs,
							const property_map<texture_ref>& version_texture_refs, property_map<texture_ref>* diff)
{
	int count = 0;
	for (const auto& [property_name, version_ref] : version_texture_refs)
	{
		const texture_ref& ancestor_ref = ancestor_texture_refs.at(property_name);
		// If they have different references ==> diff
		if (version_ref != ancestor_ref)
		{
			++count;
			if (diff) { (*diff)[property_name] = version_ref; }
		}
	}
	return count;
}

int diff_texture_references(const node& ancestor_node, const node& version_node, property_map<texture_ref>* diff)
{
	return diff_texture_references(ancestor_node.texture_references, version_node.texture_references, diff);
}

int diff_input_references(const property_map<edge>& ancestor_input_refs, const property_map<edge>& version_input_refs,
						  const ref_match<node_ref>& node_matches, property_map<edge>* diff)
{
	int count = 0;
	for (const auto& [socket_name, version_edge] : version_input_refs)
	{
		const edge& ancestor_edge = ancestor_input_refs.at(socket_name);
		if (!node_matches.has_match_in_ancestor(version_edge.node))
		{
			++count;
			if (diff) { (*diff)[socket_name] = version_edge; }
		}
		else
		{
			edge match_version_edge{.node		 = node_matches.to_ancestor(version_edge.node),
									.socket_name = version_edge.socket_name};
			if (ancestor_edge != match_version_edge)
			{
				++count;
				if (diff) { (*diff)[socket_name] = std::move(match_version_edge); }
			}
		}
	}
	return count;
}

int diff_input_references(const node& ancestor_node, const node& version_node, const ref_match<node_ref>& node_matches,
						  property_map<edge>* diff)
{
	return diff_input_references(ancestor_node.input_references, version_node.input_references, node_matches, diff);
}
}; // namespace nd

///
/// Diff functions for: nodes, graphs and scripts
///
namespace nd
{
// Nodes
node_diff diff_nodes(const node& ancestor, const node& version, const ref_match<node_ref>& node_matches,
					 const ref_match<graph_ref>& graph_matches)
{
	node_diff diff;

	// Diff node values
	diff_node_values(ancestor.node_values, version.node_values, &diff.node_values);
	// Diff node references
	diff_node_references(ancestor.node_references, version.node_references, node_matches, &diff.node_references);
	// Diff graph references
	diff_graph_references(ancestor.graph_references, version.graph_references, graph_matches, &diff.graph_references);
	// Diff node textures
	diff_texture_references(ancestor.texture_references, version.texture_references, &diff.texture_references);
	// Diff node input archs
	diff_input_references(ancestor.input_references, version.input_references, node_matches, &diff.input_references);

	return diff;
}

// Graphs
graph_diff diff_graphs(const graph& ancestor, const graph& version, const ref_match<node_ref>& node_matches,
					   const ref_match<graph_ref>& graph_matches)
{
	graph_diff diff;

	for (const auto& [version_id, version_node] : version.nodes)
	{
		// If version_node is not in the match map ==> add
		if (!node_matches.has_match_in_ancestor(version_id))
		{
			diff.nodes[version_id] = node_change{.op = diff_operation::add, .diff = version_node};
			node& partial_node	   = diff.nodes.at(version_id).diff;
			rename_node(partial_node, node_matches, graph_matches);
			continue;
		}

		// Otherwise could be an "edit"
		const node_ref& matched_version_id = node_matches.to_ancestor(version_id);
		const node& ancestor_node		   = get_node(ancestor, matched_version_id);

		node_change node_change{.op	  = diff_operation::edit,
								.diff = diff_nodes(ancestor_node, version_node, node_matches, graph_matches)};

		if (!is_empty(node_change.diff)) { diff.nodes[matched_version_id] = node_change; }
	}

	for (const auto& [ancestor_id, ancestor_node] : ancestor.nodes)
	{
		// If ancestor_node is not in version ==> delete
		if (!node_matches.has_match_in_version(ancestor_id))
		{
			diff.nodes[ancestor_id] = node_change{.op = diff_operation::del, .diff = ancestor_node};
		}
	}
	return diff;
}

// Scripts
script_diff diff_scripts(const script& ancestor, const script& version, const ref_match<graph_ref>& graph_matches)
{
	script_diff diff;
	for (const auto& [version_id, version_graph] : version.graphs)
	{
		// If version_graph is not in the rename map ==> add
		if (!graph_matches.has_match_in_ancestor(version_id))
		{
			diff.graphs[version_id] = graph_change{.op = diff_operation::add, .graph = version_graph};
			rename_graph(diff.graphs.at(version_id).graph, graph_matches);
			continue;
		}

		// Otherwise COULD be an "edit"
		const graph_ref& matched_version_id		= graph_matches.to_ancestor(version_id);
		const graph& ancestor_graph				= ancestor.graphs.at(matched_version_id);
		const ref_match<node_ref>& node_matches = match_nodes(ancestor_graph, version_graph, graph_matches);
		// Find differences between graphs
		graph_change graph_change{.op	= diff_operation::edit,
								  .diff = diff_graphs(ancestor_graph, version_graph, node_matches, graph_matches)};

		if (!is_empty(graph_change.diff)) { diff.graphs[matched_version_id] = graph_change; }
	}

	for (const auto& [ancestor_id, ancestor_graph] : ancestor.graphs)
	{
		// If ancestor_node is not in version ==> del
		if (!graph_matches.has_match_in_version(ancestor_id))
		{
			diff.graphs[ancestor_id] = graph_change{.op = diff_operation::del, .graph = ancestor_graph};
		}
	}
	return diff;
}
}; // namespace nd

///
/// Diff utility functions
///
namespace nd
{
// Check if diff is empty
bool is_empty(const node_diff& diff)
{
	return diff.node_values.empty() && diff.node_references.empty() && diff.graph_references.empty() &&
		   diff.texture_references.empty() && diff.input_references.empty();
}
bool is_empty(const graph_diff& diff) { return diff.nodes.empty(); }
bool is_empty(const script_diff& diff) { return diff.graphs.empty(); }

// Optimize (i.e. reduce) diff dimensions
void remove_common_adds(const script_diff& diff1, script_diff& diff2)
{
	for (auto& [graph_id, graph_change] : diff1.graphs)
	{
		if (graph_change.op == diff_operation::edit && diff2.graphs.contains(graph_id))
		{
			graph_diff& graph_diff2 = diff2.graphs.at(graph_id).diff;
			remove_common_adds(graph_change.diff, graph_diff2);
		}
	}
}
void remove_common_adds(const graph_diff& diff1, graph_diff& diff2)
{
	// Save all diff1's "add" operations in a vector
	std::vector<node_diff> adds = {};

	for (const auto& [node_id, node_change] : diff1.nodes)
	{
		if (node_change.op == diff_operation::add) { adds.push_back(node_change.diff); }
	}

	// For each "add" operation in diff2
	std::erase_if(diff2.nodes, [&adds](const auto& item) {
		const auto& [node_id, node_change] = item;
		return node_change.op == diff_operation::add &&
			   std::find(adds.begin(), adds.end(), node_change.diff) != adds.end();
	});
}

// Apply diffs
void apply_diff(node& node, const node_diff& diff)
{
	update(node.node_values, diff.node_values);
	update(node.node_references, diff.node_references);
	update(node.graph_references, diff.graph_references);
	update(node.texture_references, diff.texture_references);
	update(node.input_references, diff.input_references);
}
void apply_diff(graph& graph, const graph_diff& diff)
{
	for (auto& [node_id, node_change] : diff.nodes)
	{
		switch (node_change.op)
		{
		case diff_operation::add: add_node(graph, node_id, node_change.diff); break;
		case diff_operation::del: remove_node(graph, node_id); break;
		case diff_operation::edit: apply_diff(get_node(graph, node_id), node_change.diff); break;
		case diff_operation::none:
		default: assert(false && "Invalid diff operation"); break;
		}
	}
}
void apply_diff(script& script, const script_diff& diff)
{
	for (auto& [graph_id, graph_change] : diff.graphs)
	{
		switch (graph_change.op)
		{
		case diff_operation::add: add_graph(script, graph_id, graph_change.graph); break;
		case diff_operation::del: remove_graph(script, graph_id); break;
		case diff_operation::edit: apply_diff(get_graph(script, graph_id), graph_change.diff); break;
		case diff_operation::none:
		default: assert(false && "Invalid diff operation"); break;
		}
	}
}
}; // namespace nd

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
using namespace nd;

void adl_serializer<node_change>::to_json(nd::json& j, const node_change& node_change)
{
	j["operation"] = node_change.op;
	j["diff"]	   = node_change.diff;
}
void adl_serializer<node_change>::from_json(const nd::json& j, node_change& node_change)
{
	node_change.op	 = j["operation"];
	node_change.diff = j["diff"];
}

void adl_serializer<graph_diff>::to_json(nd::json& j, const graph_diff& graph_diff)
{
	j = nd::json::object();
	for (const auto& [node_id, node_change] : graph_diff.nodes)
	{
		j[node_id.name] = node_change;
	}
}
void adl_serializer<graph_diff>::from_json(const nd::json& j, graph_diff& graph_diff)
{
	for (const auto& [node_id, node_change] : j.items())
	{
		graph_diff.nodes[node_ref{.name = node_id}] = node_change;
	}
}

void adl_serializer<graph_change>::to_json(nd::json& j, const graph_change& graph_change)
{
	j["operation"] = graph_change.op;
	switch (graph_change.op)
	{
	case diff_operation::add:
	case diff_operation::del: j["diff"] = graph_change.graph; break;
	case diff_operation::edit: j["diff"] = graph_change.diff; break;
	case diff_operation::none: break;
	}
}
void adl_serializer<graph_change>::from_json(const nd::json& j, graph_change& graph_change)
{
	graph_change.op = j["operation"];
	switch (graph_change.op)
	{
	case diff_operation::add:
	case diff_operation::del: graph_change.graph = j["diff"]; break;
	case diff_operation::edit: graph_change.diff = j["diff"]; break;
	case diff_operation::none: break;
	}
}

void adl_serializer<script_diff>::to_json(nd::json& j, const script_diff& script_diff)
{
	j = nd::json::object();
	for (const auto& [graph_id, graph_change] : script_diff.graphs)
	{
		j[graph_id.name] = graph_change;
	}
}
void adl_serializer<script_diff>::from_json(const nd::json& j, script_diff& script_diff)
{
	for (const auto& [graph_id, graph_change] : j.items())
	{
		script_diff.graphs[graph_ref{.name = graph_id}] = graph_change;
	}
}
} // namespace nlohmann
