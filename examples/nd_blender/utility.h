#pragma once
#include "parser.h"

#include <array>
#include <nodediff/diff.h>
// Forward decl.
namespace nd
{
struct script;
struct graph;
struct node;
struct graph_diff;
struct script_diff;

struct graph_ref;
}; // namespace nd

namespace nd
{

namespace blender
{
	using color3 = std::array<float, 3>;
	struct visual_patch_color_schema
	{
		color3 add_color	= {0.01, 0.4, 0.03};
		color3 delete_color = {0.44, 0.06, 0.05};
		color3 edit_color	= {0.57, 0.43, 0.85};
	};

	void apply_diff_visually(graph& graph, const graph_diff& diff, const visual_patch_color_schema& color_schema = {});
	void apply_diff_visually(script& script, const script_diff& diff,
							 const visual_patch_color_schema& color_schema = {});

	void apply_merge_visually(script& script, const script_diff& diff1, const script_diff& diff2);

	void diff_ignore_node_property_values(node_diff& diff, const std::unordered_set<std::string>& ignores);
	void diff_ignore_node_property_values(graph_diff& diff, const std::unordered_set<std::string>& ignores);
	void diff_ignore_node_property_values(script_diff& diff, const std::unordered_set<std::string>& ignores);
}; // namespace blender
}; // namespace nd