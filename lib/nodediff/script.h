#pragma once
#include "reference.h"
#include "utility/types.h"
#include "value.h"

// Forward declarations
namespace nd
{
struct edge;
}; // namespace nd

namespace std
{
template <>
struct hash<nd::edge>
{
	size_t operator()(const nd::edge& edge) const;
};
}; // namespace std

///
/// Data structures
///
namespace nd
{
/*
 * Object storing edge's information.
 * In NodeGit edges are stored in a backward-manner, this means that the edge is stored in the "destination" node.
 *
 * Example: node_1:socket1 ---> node2:socket2
 * node2 will store an edge {.node=node_1, .socket_name=socket1}.
 */
struct edge
{
	// node_ref to the node from which the edge starts
	node_ref node = node_ref::invalid_ref;
	// the socket from which the edge starts
	std::string socket_name = "";

	static const edge invalid_edge;

	bool operator==(const edge& other) const;
	bool operator!=(const edge& other) const;
};

/*
 * Map of properties; for the moment it's just an std::unordered_map always using string as key type.
 */
template <typename PropertyType>
struct property_map : std::unordered_map<std::string, PropertyType>
{
	using std::unordered_map<std::string, PropertyType>::unordered_map;
};

/*
 * A node in NodeGit is modeled as a collection of properties.
 * Those properties can be either values (nd::value) or references (nd::node_ref, nd::graph_ref), in particular:
 * - node_values: store property values (i.e.: float, integer, list, ...),
 * - node_references: store references to other nodes (i.e.: a node_ref),
 * - graph_references: store references to graphs (i.e.: a graph_ref),
 * - texture_references: store references to textures (i.e.: a texture_ref),
 * - input_references: store references to other nodes; the difference with node::node_references is that here it's also
 * stored a socket information. Note: empty sockets should be set to edge::invalid_ref
 *
 * Note: in NodeGit we assume that nodes with same type will ALWAYS have same set of properties.
 */
struct node
{
	property_map<value> node_values = {};

	// References
	property_map<node_ref> node_references		 = {};
	property_map<graph_ref> graph_references	 = {};
	property_map<texture_ref> texture_references = {};
	property_map<edge> input_references			 = {};
};

/*
 * A graph in NodeGit is modeled as an unordered collection of nodes, each of which it is assigned a unique identifier
 * (namely a nd::node_ref). At the moment the unordered collection is an std::unordered_map.
 */
struct graph
{
	std::unordered_map<node_ref, node> nodes = {};
};

/*
 * A script in NodeGit represents a collection of graphs, each of which it is assigned a unique identifier (namely a
 * nd::graph_ref).
 */
struct script
{
	std::unordered_map<graph_ref, graph> graphs = {};
};
} // namespace nd

///
/// Node's functionalities
///
namespace nd
{
/*
 * Property's getters - get a node's property value/reference by property name
 */
[[nodiscard]] value& get_property_value(node& node, const std::string& property_name);
[[nodiscard]] const value& get_property_value(const node& node, const std::string& property_name);
[[nodiscard]] node_ref& get_node_reference(node& node, const std::string& property_name);
[[nodiscard]] const node_ref& get_node_reference(const node& node, const std::string& property_name);
[[nodiscard]] graph_ref& get_graph_reference(node& node, const std::string& property_name);
[[nodiscard]] const graph_ref& get_graph_reference(const node& node, const std::string& property_name);
[[nodiscard]] texture_ref& get_texture_reference(node& node, const std::string& property_name);
[[nodiscard]] const texture_ref& get_texture_reference(const node& node, const std::string& property_name);
[[nodiscard]] edge& get_input_reference(node& node, const std::string& socket_name);
[[nodiscard]] const edge& get_input_reference(const node& node, const std::string& socket_name);
/*
 * Property's adders - add new property to node with an associated value
 */
void add_property_value(node& node, const std::string& property_name, const value& value);
void add_node_reference(node& node, const std::string& property_name, const node_ref& node_reference);
void add_graph_reference(node& node, const std::string& property_name, const graph_ref& graph_reference);
void add_texture_reference(node& node, const std::string& property_name, const texture_ref& texture_reference);
void add_input_reference(node& node, const std::string& socket_name, const edge& input_reference);
/*
 * Property 's setters - set a new value to an existing node' s property.
 */
void set_property_value(node& node, const std::string& property_name, const value& value);
void set_node_reference(node& node, const std::string& property_name, const node_ref& node_reference);
void set_graph_reference(node& node, const std::string& property_name, const graph_ref& graph_reference);
void set_texture_reference(node& node, const std::string& property_name, const texture_ref& texture_reference);
void set_input_reference(node& node, const std::string& socket_name, const edge& input_reference);
/*
 * Property's removers - remove an existing property from node.
 */
void remove_property_value(node& node, const std::string& property_name);
void remove_node_reference(node& node, const std::string& property_name);
void remove_graph_reference(node& node, const std::string& property_name);
void remove_texture_reference(node& node, const std::string& property_name);
void remove_input_reference(node& node, const std::string& socket_name);

/*
 * Getter for retrieving node type (to be defined by user).
 * Node type can either be a nd::node or a subclass of it.
 * Note: it MUST be implemented in order to use the matching algorithm for nodes.
 */
template <typename Node, typename = std::enable_if_t<std::is_base_of_v<node, Node>>>
[[nodiscard]] std::string get_node_type(const Node& node);
}; // namespace nd

///
/// Graph's functionalities
///
namespace nd
{
/*
 * Graph's getters - get a node in graph by its id (a nd::node_ref)
 */
[[nodiscard]] node& get_node(graph& graph, const node_ref& node_id);
[[nodiscard]] const node& get_node(const graph& graph, const node_ref& node_id);
/*
 * Graph's adder - add a new node to graph
 */
void add_node(graph& graph, const node_ref& node_id, const node& node);
// Graph's setter - set an existing node_id to another nd::node value
void set_node(graph& graph, const node_ref& node_id, const node& node);
// Graph's remover - remove an existing node from the graph
void remove_node(graph& graph, const node_ref& node_id);
}; // namespace nd

///
/// Script's functionalities
///
namespace nd
{
/*
 * Script's getters - get a graph in script by its id (a nd::graph_ref)
 */
[[nodiscard]] graph& get_graph(script& script, const graph_ref& graph_id);
[[nodiscard]] const graph& get_graph(const script& script, const graph_ref& graph_id);
/*
 * Script's adder - add a new graph to script
 */
void add_graph(script& script, const graph_ref& graph_id, const graph& graph);
/*
 * Script's setter - set an existing graph_id to another nd::graph value
 */
void set_graph(script& script, const graph_ref& graph_id, const graph& graph);
/*
 * Script's remover - remove an existing graph from the script
 */
void remove_graph(script& script, const graph_ref& graph_id);
}; // namespace nd

///
///	STL
///
namespace std
{
ostream& operator<<(ostream& os, const nd::edge& edge);
ostream& operator<<(ostream& os, const nd::node& node);
} // namespace std

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
template <>
struct adl_serializer<nd::edge>
{
	static void to_json(nd::json& j, const nd::edge& edge);
	static void from_json(const nd::json& j, nd::edge& edge);
};
template <>
struct adl_serializer<nd::node>
{
	static void to_json(nd::json& j, const nd::node& node);
	static void from_json(const nd::json& j, nd::node& node);
};

template <>
struct adl_serializer<nd::graph>
{
	static void to_json(nd::json& j, const nd::graph& graph);
	static void from_json(const nd::json& j, nd::graph& graph);
};

template <>
struct adl_serializer<nd::script>
{
	static void to_json(nd::json& j, const nd::script& script);
	static void from_json(const nd::json& j, nd::script& script);
};
} // namespace nlohmann