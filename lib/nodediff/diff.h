#pragma once
#include "matching.h"
#include "script.h"

///
/// Data structures
///
namespace nd
{
/*
 * Diff operation enum, is general for:
 *	- add: adding a graph/node in a script/graph
 *	- del: deleting a graph/node in a script/graph
 *	- edit: editing a graph in a script (add/del/edit of its nodes) or editing of node's properties
 *	- none: empty diff
 */
enum struct diff_operation
{
	add,
	del,
	edit,
	none
};

/*
 * A node_diff is modeled as a partial node, only storing changed properties with their new values.
 */
typedef node node_diff;

/*
 * A node_change is modeled as:
 *	- node_change::op the change operation (addition/deletion/edit of a node)
 *	- node_change::diff the partial/entire node. In case of addition or deletion it's a complete node; in case of edit
						it only contains changed properties.
 */
struct node_change
{
	diff_operation op = diff_operation::none;
	node_diff diff	  = {};
};

/*
 * A graph_diff is modeled as a collection of node_change(s), to each of which it's associated the identifier of the
 * node subject to that change.
 */
struct graph_diff
{
	std::unordered_map<node_ref, node_change> nodes = {};
};

/*
 * A graph_change is modeled as:
 *	- graph_change::op the change operation (addition/deletion/edit of a graph)
 *	- graph_change::graph the entire graph; this value is set only if graph_change::op is del or add.
 *	- graph_change::diff the partial graph; this value is set only if graph_change::op is edit
 */
struct graph_change
{
	diff_operation op = diff_operation::none;
	nd::graph graph	  = {};
	graph_diff diff	  = {};
};

/*
 * A graph_diff is modeled as a collection of graph_change(s), to each of which it's associated the identifier of the
 * graph subject to that change.
 */
struct script_diff
{
	std::unordered_map<graph_ref, graph_change> graphs = {};
};
}; // namespace nd

///
/// Operators
///
namespace nd
{
bool operator==(const node_diff& node_diff1, const node_diff& node_diff2);
bool operator==(const node_change& node_change1, const node_change& node_change2);
}; // namespace nd

///
/// Functions for diffing node's properties
///
namespace nd
{

/*
 * Diff node's property values.
 * Function parameters:
 *	- ancestor_values: ancestor property map of values
 *	- version_values: version property map of values
 *	- diff: pointer to an empty property_map of values; if set, this function will store changed properties in the
			pointed map.
 * Returns: the number of property values that are different between ancestor and version.
 */
int diff_node_values(const property_map<value>& ancestor_values, const property_map<value>& version_values,
					 property_map<value>* diff = nullptr);
/*
 * Diff node's property values; interface function for the other nd::diff_node_values function.
 */
int diff_node_values(const node& ancestor_node, const node& version_node, property_map<value>* diff = nullptr);

/*
 * Diff node's property node references.
 * Function parameters:
 *	- ancestor_node_references: ancestor property map of node references
 *	- version_node_references: version property map of node references
 *	- diff: pointer to an empty property_map of node references; if set, this function will store changed properties in
the pointed map.
* Returns: the number of property node references that are different between ancestor and version.
 */
int diff_node_references(const property_map<node_ref>& ancestor_node_references,
						 const property_map<node_ref>& version_node_references, const ref_match<node_ref>& node_matches,
						 property_map<node_ref>* diff = nullptr);
/*
 * Diff node's node references; interface function for the other nd::diff_node_references function.
 */
int diff_node_references(const node& ancestor_node, const node& version_node, const ref_match<node_ref>& node_matches,
						 property_map<node_ref>* diff = nullptr);

/*
 * Diff node's property graphs references.
 * Function parameters:
 *	- ancestor_graph_references: ancestor property map of graph references
 *	- ancestor_graph_references: version property map of graph references
 *	- diff: pointer to an empty property_map of graph references; if set, this function will store changed properties in
the pointed map.
* Returns: the number of property graph references that are different between ancestor and version.
 */
int diff_graph_references(const property_map<graph_ref>& ancestor_graph_references,
						  const property_map<graph_ref>& version_graph_references,
						  const ref_match<graph_ref>& graph_matches, property_map<graph_ref>* diff = nullptr);
/*
 * Diff node's graph references; interface function for the other nd::diff_graph_references function.
 */
int diff_graph_references(const node& ancestor_node, const node& version_node,
						  const ref_match<graph_ref>& graph_matches, property_map<graph_ref>* diff = nullptr);

/*
 * Diff node's property references to textures (a.t.m. not done properly, i.e. we are not using texture matches).
 * Function parameters:
 *	- ancestor_texture_references: ancestor property map of texture references
 *	- version_texture_references: version property map of texture references
 *	- diff: pointer to an empty property_map of texture references; if set, this function will store changed properties
			in the pointed map.
 * Returns: the number of property texture references that are different between ancestor and version.
 */
int diff_texture_references(const property_map<texture_ref>& ancestor_texture_references,
							const property_map<texture_ref>& version_texture_references,
							property_map<texture_ref>* diff = nullptr);
/*
 * Diff node's texture references; interface function for the other nd::diff_texture_references function.
 */
int diff_texture_references(const node& ancestor_node, const node& version_node,
							property_map<texture_ref>* diff = nullptr);

/*
 * Diff node's edges.
 * Function parameters:
 *	- ancestor_input_references: ancestor property map of input references
 *	- version_input_references: version property map of input references
 *	- diff: pointer to an empty property_map of input references; if set, this function will store changed properties
			in the pointed map.
 * Returns: the number of property input references that are different between ancestor and version.
 */
int diff_input_references(const property_map<edge>& ancestor_input_references,
						  const property_map<edge>& version_input_references, const ref_match<node_ref>& node_matches,
						  property_map<edge>* diff = nullptr);
/*
 * Diff node's input references; interface function for the other nd::diff_input_references function.
 */
int diff_input_references(const node& ancestor_node, const node& version_node, const ref_match<node_ref>& node_matches,
						  property_map<edge>* diff = nullptr);
}; // namespace nd

///
/// Diff functions for: nodes, graphs and scripts
///
namespace nd
{
/*
 * Diff nodes.
 * Function parameters:
 *	- ancestor_node: ancestor node
 *	- version_node: version node
 *	- node_matches: bidirectional map of matched nodes
 *	- graph_matches: bidirectional map of matched graphs
 * Returns: the diff between ancestor and version nodes
 */
[[nodiscard]] node_diff diff_nodes(const node& ancestor_node, const node& version_node,
								   const ref_match<node_ref>& node_matches, const ref_match<graph_ref>& graph_matches);
/*
 * Diff graphs.
 * Function parameters:
 *	- ancestor: ancestor graph
 *	- version: version graph
 *	- node_matches: bidirectional map of matched nodes
 *	- graph_matches: bidirectional map of matched graphs
 * Returns: the diff between ancestor and version graphs
 */
[[nodiscard]] graph_diff diff_graphs(const graph& ancestor, const graph& version,
									 const ref_match<node_ref>& node_matches,
									 const ref_match<graph_ref>& graph_matches);
/*
 * Diff scripts.
 * Function parameters:
 *	- ancestor: ancestor script
 *	- version: version script
 *	- graph_matches: bidirectional map of matched graphs
 * Returns: the diff between ancestor and version scripts
 */
[[nodiscard]] script_diff diff_scripts(const script& ancestor, const script& version,
									   const ref_match<graph_ref>& graph_matches);
}; // namespace nd

///
/// Diff utility functions
///
namespace nd
{
// Check if diff is empty (i.e. does not contain any change)
[[nodiscard]] bool is_empty(const node_diff& node_diff);
[[nodiscard]] bool is_empty(const graph_diff& graph_diff);
[[nodiscard]] bool is_empty(const script_diff& script_diff);

// Optimize (i.e. reduce) diff dimensions by removing common added nodes. Note: maybe it could be implemented also for
// graphs
void remove_common_adds(const script_diff& script_diff1, script_diff& script_diff2);
void remove_common_adds(const graph_diff& graph_diff1, graph_diff& graph_diff2);

/*
 * Apply a diff's changes to a node. It consists in updating property values with the new ones.
 * Function parameters:
 *	- node: node on to which apply changes
 *	- node_diff: properties to update
 */
void apply_diff(node& node, const node_diff& node_diff);

/*
 * Apply a diff's changes to a graph. It consists in:
 *	- adding new nodes
 *	- deleting existing nodes
 *	- editing nodes by updating their properties using nd::apply_diff for nodes.
 * 
 * Function parameters:
 *	- graph: graph on to which apply changes
 *	- graph_diff: set of node_change(s) to apply
 */
void apply_diff(graph& graph, const graph_diff& graph_diff);

/*
 * Apply a diff's changes to a script. It consists in:
 *	- adding new graphs
 *	- deleting existing graphs
 *	- editing graphs by updating its nodes using nd::apply_diff for graphs.
 *
 * Function parameters:
 *	- graph: graph on to which apply changes
 *	- graph_diff: set of node_change(s) to apply
 */
void apply_diff(script& script, const script_diff& script_diff);
}; // namespace nd

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
NLOHMANN_JSON_SERIALIZE_ENUM(nd::diff_operation, {{nd::diff_operation::none, "none"},
												  {nd::diff_operation::add, "add"},
												  {nd::diff_operation::del, "del"},
												  {nd::diff_operation::edit, "edit"}});
template <>
struct adl_serializer<nd::node_change>
{
	static void to_json(nd::json& j, const nd::node_change& node_change);
	static void from_json(const nd::json& j, nd::node_change& node_change);
};

template <>
struct adl_serializer<nd::graph_diff>
{
	static void to_json(nd::json& j, const nd::graph_diff& graph_diff);
	static void from_json(const nd::json& j, nd::graph_diff& graph_diff);
};

template <>
struct adl_serializer<nd::graph_change>
{
	static void to_json(nd::json& j, const nd::graph_change& graph_change);
	static void from_json(const nd::json& j, nd::graph_change& graph_change);
};

template <>
struct adl_serializer<nd::script_diff>
{
	static void to_json(nd::json& j, const nd::script_diff& script_diff);
	static void from_json(const nd::json& j, nd::script_diff& script_diff);
};
} // namespace nlohmann
