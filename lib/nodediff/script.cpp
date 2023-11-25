#include "script.h"

#include "utility/utility.h"

///
/// Static member init.
///
namespace nd
{
const node_ref node_ref::invalid_ref   = {.name = ""};
const graph_ref graph_ref::invalid_ref = {.name = ""};
const edge edge::invalid_edge		   = {.node = node_ref::invalid_ref, .socket_name = ""};
} // namespace nd

// edge implementations
namespace nd
{
bool edge::operator==(const edge& other) const
{
	return this->node == other.node && this->socket_name == other.socket_name;
}
bool edge::operator!=(const edge& other) const { return !(*this == other); }
}; // namespace nd

///
/// Node's functionalities
///
namespace nd
{
value& get_property_value(node& node, const std::string& property_name) { return node.node_values.at(property_name); }
const value& get_property_value(const node& node, const std::string& property_name)
{
	return node.node_values.at(property_name);
}
node_ref& get_node_reference(node& node, const std::string& property_name)
{
	return node.node_references.at(property_name);
}
const node_ref& get_node_reference(const node& node, const std::string& property_name)
{
	return node.node_references.at(property_name);
}
graph_ref& get_graph_reference(node& node, const std::string& property_name)
{
	return node.graph_references.at(property_name);
}
const graph_ref& get_graph_reference(const node& node, const std::string& property_name)
{
	return node.graph_references.at(property_name);
}

texture_ref& get_texture_reference(node& node, const std::string& property_name)
{
	return node.texture_references.at(property_name);
}
const texture_ref& get_texture_reference(const node& node, const std::string& property_name)
{
	return node.texture_references.at(property_name);
}
edge& get_input_reference(node& node, const std::string& socket_name) { return node.input_references.at(socket_name); }
const edge& get_input_reference(const node& node, const std::string& socket_name)
{
	return node.input_references.at(socket_name);
}
void add_property_value(node& node, const std::string& property_name, const value& value)
{
	node.node_values[property_name] = value;
}
void add_node_reference(node& node, const std::string& property_name, const node_ref& reference)
{
	node.node_references[property_name] = reference;
}
void add_graph_reference(node& node, const std::string& property_name, const graph_ref& reference)
{
	node.graph_references[property_name] = reference;
}
void add_texture_reference(node& node, const std::string& property_name, const texture_ref& reference)
{
	node.texture_references[property_name] = reference;
}
void add_input_reference(node& node, const std::string& socket_name, const edge& reference)
{
	node.input_references[socket_name] = reference;
}
void set_property_value(node& node, const std::string& property_name, const value& value)
{
	node.node_values.at(property_name) = value;
}
void set_node_reference(node& node, const std::string& property_name, const node_ref& reference)
{
	node.node_references.at(property_name) = reference;
}
void set_graph_reference(node& node, const std::string& property_name, const graph_ref& reference)
{
	node.graph_references.at(property_name) = reference;
}
void set_texture_reference(node& node, const std::string& property_name, const texture_ref& reference)
{
	node.texture_references.at(property_name) = reference;
}
void set_input_reference(node& node, const std::string& socket_name, const edge& reference)
{
	node.input_references.at(socket_name) = reference;
}
void remove_property_value(node& node, const std::string& property_name)
{
	assert(node.node_values.contains(property_name) && "Trying to remove a property value which does not exist");
	node.node_values.erase(property_name);
}
void remove_node_reference(node& node, const std::string& property_name)
{
	assert(node.node_references.contains(property_name) && "Trying to remove a node reference which does not exist");
	node.node_references.erase(property_name);
}
void remove_graph_reference(node& node, const std::string& property_name)
{
	assert(node.graph_references.contains(property_name) && "Trying to remove a graph reference which does not exist");
	node.graph_references.erase(property_name);
}
void remove_texture_reference(node& node, const std::string& property_name)
{
	assert(node.texture_references.contains(property_name) &&
		   "Trying to remove a texture reference which does not exist");
	node.texture_references.erase(property_name);
}
void remove_input_reference(node& node, const std::string& socket_name)
{
	assert(node.texture_references.contains(socket_name) && "Trying to remove an input reference which does not exist");
	node.input_references.erase(socket_name);
}
}; // namespace nd

///
/// Graph's functionalities
///
namespace nd
{
node& get_node(graph& graph, const node_ref& node_id) { return graph.nodes.at(node_id); }
const node& get_node(const graph& graph, const node_ref& node_id) { return graph.nodes.at(node_id); }
void add_node(graph& graph, const node_ref& node_id, const node& node) { graph.nodes[node_id] = node; }
void set_node(graph& graph, const node_ref& node_id, const node& node) { graph.nodes.at(node_id) = node; }
void remove_node(graph& graph, const node_ref& node_id)
{
	assert(graph.nodes.contains(node_id) && "Trying to remove a node which does not exist");
	graph.nodes.erase(node_id);
}
}; // namespace nd

/// 
/// Script's functionalities
/// 
namespace nd
{
graph& get_graph(script& script, const graph_ref& graph_id) { return script.graphs.at(graph_id); }
const graph& get_graph(const script& script, const graph_ref& graph_id) { return script.graphs.at(graph_id); }
void add_graph(script& script, const graph_ref& graph_id, const graph& graph) { script.graphs[graph_id] = graph; }
void set_graph(script& script, const graph_ref& graph_id, const graph& graph) { script.graphs.at(graph_id) = graph; }
void remove_graph(script& script, const graph_ref& graph_id)
{
	assert(script.graphs.contains(graph_id) && "Trying to remove a graph which does not exist");
	script.graphs.erase(graph_id);
}
}; // namespace nd

///
///	STL
///
namespace std
{
size_t hash<nd::edge>::operator()(const nd::edge& edge) const
{
	size_t seed = 0;
	nd::hash_combine(seed, edge.node, edge.socket_name);
	return seed;
}

ostream& operator<<(ostream& os, const nd::edge& edge)
{
	os << nd::json(edge);
	return os;
}
ostream& operator<<(ostream& os, const nd::node& node)
{
	os << nd::json(node);
	return os;
}
} // namespace std

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
using namespace nd;

void adl_serializer<edge>::to_json(nd::json& j, const edge& edge)
{
	j["node"]	= edge.node;
	j["socket"] = edge.socket_name;
}
void adl_serializer<edge>::from_json(const nd::json& j, edge& edge)
{
	edge.node		 = j["node"];
	edge.socket_name = j["socket"];
}

void adl_serializer<node>::to_json(nd::json& j, const node& node)
{
	j["node_values"]		= node.node_values;
	j["node_references"]	= node.node_references;
	j["graph_references"]	= node.graph_references;
	j["texture_references"] = node.texture_references;
	j["input_references"]	= node.input_references;
}
void adl_serializer<node>::from_json(const nd::json& j, node& node)
{
	node.node_values		= j["node_values"];
	node.node_references	= j["node_references"];
	node.graph_references	= j["graph_references"];
	node.texture_references = j["texture_references"];
	node.input_references	= j["input_references"];
}

void adl_serializer<graph>::to_json(nd::json& j, const graph& graph)
{
	j = nd::json::object();
	for (const auto& [node_id, node] : graph.nodes)
	{
		j[node_id.name] = node;
	}
}
void adl_serializer<graph>::from_json(const nd::json& j, graph& graph)
{
	for (const auto& [node_id, node] : j.items())
	{
		add_node(graph, node_ref{.name = node_id}, node);
	}
}

void adl_serializer<script>::to_json(nd::json& j, const script& script)
{
	j = nd::json::object();
	for (const auto& [graph_id, graph] : script.graphs)
	{
		j[graph_id.name] = graph;
	}
}
void adl_serializer<script>::from_json(const nd::json& j, script& script)
{
	for (const auto& [graph_id, graph] : j.items())
	{
		add_graph(script, graph_ref{.name = graph_id}, graph);
	}
}
} // namespace nlohmann
