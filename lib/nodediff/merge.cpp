#include "merge.h"

#include "diff.h"
#include "utility/utility.h"

namespace nd
{
bool check_diff_conflicts(const graph_diff& diff1, const graph_diff& diff2,
						  out_var std::vector<node_conflict>& conflicts)
{
	// Check for merge conflicts
	for (const auto& [node_id1, node_change1] : diff1.nodes)
	{
		if (diff2.nodes.contains(node_id1))
		{
			const node_change& node_change2 = diff2.nodes.at(node_id1);
			// in one version we have a delete on a node X and on the other we edit it ==> merge conflict
			if (node_change1.op == diff_operation::del && node_change2.op == diff_operation::edit)
			{
				conflicts.emplace_back(node_conflict{.type_v = node_conflict::type::del_edit, .node = node_id1});
			}
			if ((node_change2.op == diff_operation::del && node_change1.op == diff_operation::edit))
			{
				conflicts.emplace_back(node_conflict{.type_v = node_conflict::type::edit_del, .node = node_id1});
			}

			// same property set (with different values) in both versions ==> merge conflict
			if (node_change1.op == diff_operation::edit && node_change2.op == diff_operation::edit)
			{
				std::vector<std::string> conflicting_properties, conflicting_edges;
				// By-Value properties
				for (const auto& [property_name, value] : node_change1.diff.node_values)
				{
					if (node_change2.diff.node_values.contains(property_name) &&
						node_change2.diff.node_values.at(property_name) != value)
					{
						// Merge conflict
						conflicting_properties.emplace_back(property_name);
					}
				}

				// Node-references properties
				for (const auto& [property_name, node_reference] : node_change1.diff.node_references)
				{
					if (node_change2.diff.node_references.contains(property_name) &&
						node_change2.diff.node_references.at(property_name) != node_reference)
					{
						// Merge conflict
						conflicting_properties.emplace_back(property_name);
					}
				}

				// Graph-references properties
				for (const auto& [property_name, graph_reference] : node_change1.diff.graph_references)
				{
					if (node_change2.diff.graph_references.contains(property_name) &&
						node_change2.diff.graph_references.at(property_name) != graph_reference)
					{
						// Merge conflict
						conflicting_properties.emplace_back(property_name);
					}
				}

				// Texture-references properties
				for (const auto& [property_name, texture_reference] : node_change1.diff.texture_references)
				{
					if (node_change2.diff.texture_references.contains(property_name) &&
						node_change2.diff.texture_references.at(property_name) != texture_reference)
					{
						// Merge conflict
						conflicting_properties.emplace_back(property_name);
					}
				}

				// Input references
				for (const auto& [socket_name, input_references] : node_change1.diff.input_references)
				{
					if (node_change2.diff.input_references.contains(socket_name) &&
						node_change2.diff.input_references.at(socket_name) != input_references)
					{
						// Merge conflict
						conflicting_edges.emplace_back(socket_name);
					}
				}
				if (!conflicting_properties.empty() || !conflicting_edges.empty())
				{
					conflicts.emplace_back(node_conflict{.type_v	 = node_conflict::type::edit_edit,
														 .node		 = node_id1,
														 .properties = conflicting_properties,
														 .edges		 = conflicting_edges});
				}
			}
		}
	}
	return conflicts.size() > 0;
}

bool check_diff_conflicts(const script_diff& diff1, const script_diff& diff2,
						  out_var std::vector<graph_conflict>& conflicts)
{
	for (const auto& [graph_id1, graph_change1] : diff1.graphs)
	{
		if (diff2.graphs.contains(graph_id1))
		{
			const graph_change& graph_change2 = diff2.graphs.at(graph_id1);
			// in one version we have a delete on a graph X and on the other we edit it
			if (graph_change1.op == diff_operation::del && graph_change2.op == diff_operation::edit)
			{
				conflicts.emplace_back(graph_conflict{.type_v = graph_conflict::type::del_edit, .graph = graph_id1});
			}
			if (graph_change2.op == diff_operation::del && graph_change1.op == diff_operation::edit)
			{
				conflicts.emplace_back(graph_conflict{.type_v = graph_conflict::type::edit_del, .graph = graph_id1});
			}

			// editing the same graph && the edits are conflicting ==> merge conflict
			if (graph_change1.op == diff_operation::edit && graph_change2.op == diff_operation::edit)
			{
				graph_conflict conflict;
				if (check_diff_conflicts(graph_change1.diff, graph_change2.diff, conflict.nodes))
				{
					conflict.type_v = graph_conflict::type::edit_edit;
					conflict.graph	= graph_id1;
					conflicts.emplace_back(std::move(conflict));
				}
			}
		}
	}
	return conflicts.size() > 0;
}

graph_merge_result merge_graphs(const graph& ancestor, const graph_diff& diff1, const graph_diff& diff2)
{
	graph_merge_result result{.result = ancestor};
	if (!check_diff_conflicts(diff1, diff2, result.conflicts))
	{
		apply_diff(result.result, diff1);
		apply_diff(result.result, diff2);
	}
	return result;
}

script_merge_result merge_scripts(const script& ancestor, const script_diff& diff1, const script_diff& diff2)
{
	script_merge_result result{.result = ancestor};

	// Merge if there are no conflicts
	if (!check_diff_conflicts(diff1, diff2, result.conflicts))
	{
		apply_diff(result.result, diff1);
		apply_diff(result.result, diff2);
	}
	return result;
}
} // namespace nd

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
void adl_serializer<nd::node_conflict>::to_json(nd::json& j, const nd::node_conflict& node_conflict)
{
	j["type"]		= node_conflict.type_v;
	j["node"]		= node_conflict.node;
	j["properties"] = node_conflict.properties;
	j["edges"]		= node_conflict.edges;
}
void adl_serializer<nd::node_conflict>::from_json(const nd::json& j, nd::node_conflict& node_conflict)
{
	node_conflict.type_v	 = j["type"];
	node_conflict.node		 = j["node"];
	node_conflict.properties = j["properties"].get<std::vector<std::string>>();
	node_conflict.edges		 = j["edges"].get<std::vector<std::string>>();
}

void adl_serializer<nd::graph_conflict>::to_json(nd::json& j, const nd::graph_conflict& graph_conflict)
{
	j["type"]  = graph_conflict.type_v;
	j["graph"] = graph_conflict.graph;
	j["nodes"] = graph_conflict.nodes;
}

void adl_serializer<nd::graph_conflict>::from_json(const nd::json& j, nd::graph_conflict& graph_conflict)
{
	graph_conflict.type_v = j["type"];
	graph_conflict.graph  = j["graph"];
	graph_conflict.nodes  = j["nodes"];
}

void adl_serializer<nd::graph_merge_result>::to_json(nd::json& j, const nd::graph_merge_result& graph_merge_result)
{
	j["result"]	   = graph_merge_result.result;
	j["conflicts"] = graph_merge_result.conflicts;
}

void adl_serializer<nd::graph_merge_result>::from_json(const nd::json& j, nd::graph_merge_result& graph_merge_result)
{
	graph_merge_result.result	 = j["result"];
	graph_merge_result.conflicts = j["conflicts"];
}

void adl_serializer<nd::script_merge_result>::to_json(nd::json& j, const nd::script_merge_result& script_merge_result)
{
	j["result"]	   = script_merge_result.result;
	j["conflicts"] = script_merge_result.conflicts;
}

void adl_serializer<nd::script_merge_result>::from_json(const nd::json& j, nd::script_merge_result& script_merge_result)
{
	script_merge_result.result	  = j["result"];
	script_merge_result.conflicts = j["conflicts"];
}
} // namespace nlohmann