#include "utility.h"

#include "parser.h"

#include <nodediff/diff.h>

namespace nd
{
template <>
std::string get_node_type(const node& node)
{
	const std::string& node_type = get_property_value(node, blender::NODE_TYPE).get<std::string>();
	if (node_type == "ShaderNodeGroup" || node_type == "GeometryNodeGroup")
	{
		return get_property_value(node, "p.group_name").get<std::string>();
	}
	return node_type;
}
namespace blender
{

	void color_node(node& node, const color3& color)
	{
		add_property_value(node, "a.use_custom_color", value(1));
		add_property_value(node, "a.color", value(std::vector<float>{color[0], color[1], color[2]}));
	}

	void apply_diff_visually(graph& graph, const graph_diff& diff, const visual_patch_color_schema& color_schema)
	{
		for (auto& [node_id, node_change] : diff.nodes)
		{
			if (node_change.op == diff_operation::del) { graph.nodes[node_id] = node_change.diff; }
			node& node = get_node(graph, node_id);

			// Skip virtual node for interface_inputs
			if (get_property_value(node, NODE_TYPE) == nkit::NODETREE_INTERFACE_INPUTS) continue;

			switch (node_change.op)
			{
			case diff_operation::add: color_node(node, color_schema.add_color); break;
			case diff_operation::del:
				color_node(node, color_schema.delete_color);
				node.input_references = {};
				break;
			case diff_operation::edit: color_node(node, color_schema.edit_color); break;
			case diff_operation::none: assert(false && "Invalid diff operation"); break;
			}
		}
	}

	void apply_diff_visually(script& script, const script_diff& diff, const visual_patch_color_schema& color_schema)
	{
		for (auto& [graph_id, graph_change] : diff.graphs)
		{
			switch (graph_change.op)
			{
			case diff_operation::add: break;
			case diff_operation::del: break;
			case diff_operation::edit:
				apply_diff_visually(get_graph(script, graph_id), graph_change.diff, color_schema);
				break;
			}
		}
	}

	void apply_merge_visually(script& script, const script_diff& diff1, const script_diff& diff2)
	{
		apply_diff_visually(script, diff1);
		visual_patch_color_schema secondary_cs	= {.add_color	 = color3{1, 0.88, 0.39},
												   .delete_color = color3{0.86, 0.45, 0.21},
												   .edit_color	 = color3{0.53, 0.82, 0.97}};
		visual_patch_color_schema concurrent_cs = {.add_color	 = color3{0.8, 1.0, 0.0},
												   .delete_color = color3{1.0, 0.0, 0.8},
												   .edit_color	 = color3{0.0, 0.0, 1.0}};
		apply_diff_visually(script, diff2, secondary_cs);

		for (const auto& [graph_id, graph_change] : diff1.graphs)
		{
			if (graph_change.op == diff_operation::edit && diff2.graphs.contains(graph_id))
			{
				graph& graph				  = get_graph(script, graph_id);
				const graph_diff& graph_diff1 = graph_change.diff;
				const graph_diff& graph_diff2 = diff2.graphs.at(graph_id).diff;
				for (const auto& [node_id, node_change] : graph_diff1.nodes)
				{
					if (graph_diff2.nodes.contains(node_id))
					{
						node& node = get_node(graph, node_id);
						switch (node_change.op)
						{
						case diff_operation::add: color_node(node, concurrent_cs.add_color); break;
						case diff_operation::del: color_node(node, concurrent_cs.delete_color); break;
						case diff_operation::edit: color_node(node, concurrent_cs.edit_color); break;
						case diff_operation::none:
							assert(false && "Invalid diff operation. Diff operation cannot be none in a diff");
							break;
						}
					}
				}
			}
		}
	}

	void diff_ignore_node_property_values(node_diff& diff, const std::unordered_set<std::string>& ignores)
	{
		for (const std::string& property_name : ignores)
		{
			if (diff.node_values.contains(property_name)) { diff.node_values.erase(property_name); }
		}
	}

	void diff_ignore_node_property_values(graph_diff& diff, const std::unordered_set<std::string>& ignores)
	{
		std::vector<node_ref> to_delete;
		for (auto& [node_id, node_change] : diff.nodes)
		{
			if (node_change.op == diff_operation::edit)
			{
				diff_ignore_node_property_values(node_change.diff, ignores);
				// Check if it's an empty diff
				if (is_empty(node_change.diff)) { to_delete.push_back(node_id); }
			}
		}
		for (const node_ref& node_id : to_delete)
		{
			diff.nodes.erase(node_id);
		}
	}

	void diff_ignore_node_property_values(script_diff& diff, const std::unordered_set<std::string>& ignores)
	{
		std::vector<graph_ref> to_delete;
		for (auto& [graph_id, graph_change] : diff.graphs)
		{
			if (graph_change.op == diff_operation::edit)
			{
				diff_ignore_node_property_values(graph_change.diff, ignores);
				if (is_empty(diff)) { to_delete.push_back(graph_id); }
			}
		}
		for (const graph_ref& graph_id : to_delete)
		{
			diff.graphs.erase(graph_id);
		}
	}
}; // namespace blender
}; // namespace nd