#pragma once
#include "script.h"
#include "utility/macro.h"
#include "utility/utility.h"

// Forward declarations
namespace nd
{
struct script;
struct script_diff;

struct graph;
struct graph_diff;

struct graph_ref;
struct node_ref;
}; // namespace nd

namespace nd
{

/*
 * A node conflict is modeled as a struct containing the following informations:
 * - node_conflict::type_v: node conflict type,
 * - node_conflict::node: the id of the conflicted node,
 * - node_conflict::properties: a list of properties which are conflicting (note: it's non-empty only if
 * node_conflict::type_v == edit_edit),
 * - node_conflict::edges: a list of edges which are conflicting, each element corresponds to a socket_name which is
 * conflicting (note: it's non-empty only if node_conflict::type_v == edit_edit).
 */
struct node_conflict
{
	enum struct type
	{
		del_edit, // version 1 deletes the node, version 2 edits it
		edit_del, // version 1 edits the node, version 2 deletes it
		edit_edit // both version have edited at least one identical property but with different values
	} type_v;

	node_ref node						= node_ref::invalid_ref;
	std::vector<std::string> properties = {};
	std::vector<std::string> edges		= {};
};

/*
 * A graph conflict is modeled as a struct containing the following informations:
 * - graph_conflict::type_v: graph conflict type,
 * - graph_conflict::graph: the id of the conflicted graph,
 * - graph_conflict::nodes: a list of nodes which are conflicting
							(note: it's non-empty only if graph_conflict::type_v == edit_edit),
 */
struct graph_conflict
{
	enum struct type
	{
		del_edit, // version 1 deletes the graph, version 2 edits it
		edit_del, // version 1 edits the graph, version 2 deletes it
		edit_edit // both version have edited the same graph causing at least a node_conflict
	} type_v;

	graph_ref graph					 = graph_ref::invalid_ref;
	std::vector<node_conflict> nodes = {};
};

/*
 * This struct model the result of a merge operation of two graphs.
 * If a merge succeded:
 *	1- graph_merge_result::result contains the merged graph
 *	2- graph_merge_result::conflicts is empty
 * If, instead, the merge failed script_merge_result::conflicts is non empty (and graph_merge_result::result is a dirt
	value, should not be read).
 */
struct graph_merge_result
{
	graph result						 = {};
	std::vector<node_conflict> conflicts = {};
};

/*
 * This struct model the result of a merge operation of two scripts.
 * If a merge succeded:
 *	1- script_merge_result::result contains the merged script
 *	2- script_merge_result::conflicts is empty
 * If, instead, the merge failed, script_merge_result::conflicts is non empty (and graph_merge_result::result is a dirt
	value, should not be read).
 */
struct script_merge_result
{
	script result						  = {};
	std::vector<graph_conflict> conflicts = {};
};

/*
 * Given two nd::graph_diff, it returns true if there are conflicting changes.
 * Note: node_conflicts is an std::vector<nd::node_conflict> that at the end of this function execution will contain all
 * the node conflicts found.
 */
bool check_diff_conflicts(const graph_diff& graph_diff1, const graph_diff& graph_diff2,
						  out_var std::vector<node_conflict>& node_conflicts);
/*
 * Given two nd::script_diff, it returns true if there are conflicting changes.
 * Note: graph_conflicts is an std::vector<nd::graph_conflict> that at the end of this function execution will contain
 * all the graph conflicts found.
 */
bool check_diff_conflicts(const script_diff& script_diff1, const script_diff& script_diff2,
						  out_var std::vector<graph_conflict>& graph_conflicts);

/*
 * Given a merge result it returns true if the merge cannot be performed due to a conflict
 */
[[nodiscard]] inline bool merge_has_failed(const script_merge_result& script_merge_result)
{
	return !script_merge_result.conflicts.empty();
}
/*
 * Given a merge result it returns true if the merge cannot be performed due to a conflict
 */
[[nodiscard]] inline bool merge_has_failed(const graph_merge_result& graph_merge_result)
{
	return !graph_merge_result.conflicts.empty();
}

/*
 * Given an ancestor graph, and two graph_diff(s) obtained by diffing the ancestor with two versions,
 * this function performs a three-way merge between the ancestor and the two diffed versions.
 */
[[nodiscard]] graph_merge_result merge_graphs(const graph& ancestor, const graph_diff& graph_diff1,
											  const graph_diff& graph_diff2);
/*
 * Given an ancestor script, and two script_diff(s) obtained by diffing the ancestor with two versions,
 * this function performs a three-way merge between the ancestor and the two diffed versions.
 */
[[nodiscard]] script_merge_result merge_scripts(const script& ancestor, const script_diff& script_diff1,
												const script_diff& script_diff2);
}; // namespace nd

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
NLOHMANN_JSON_SERIALIZE_ENUM(nd::node_conflict::type, {{nd::node_conflict::type::del_edit, "del_edit"},
													   {nd::node_conflict::type::edit_del, "edit_del"},
													   {nd::node_conflict::type::edit_edit, "edit_edit"}});
NLOHMANN_JSON_SERIALIZE_ENUM(nd::graph_conflict::type, {{nd::graph_conflict::type::del_edit, "del_edit"},
														{nd::graph_conflict::type::edit_del, "edit_del"},
														{nd::graph_conflict::type::edit_edit, "edit_edit"}});

template <>
struct adl_serializer<nd::node_conflict>
{
	static void to_json(nd::json& j, const nd::node_conflict& node_conflict);
	static void from_json(const nd::json& j, nd::node_conflict& node_conflict);
};

template <>
struct adl_serializer<nd::graph_conflict>
{
	static void to_json(nd::json& j, const nd::graph_conflict& graph_conflict);
	static void from_json(const nd::json& j, nd::graph_conflict& graph_conflict);
};

template <>
struct adl_serializer<nd::graph_merge_result>
{
	static void to_json(nd::json& j, const nd::graph_merge_result& graph_merge_result);
	static void from_json(const nd::json& j, nd::graph_merge_result& graph_merge_result);
};

template <>
struct adl_serializer<nd::script_merge_result>
{
	static void to_json(nd::json& j, const nd::script_merge_result& script_merge_result);
	static void from_json(const nd::json& j, nd::script_merge_result& script_merge_result);
};
}; // namespace nlohmann