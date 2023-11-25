#include "reference.h"

///
/// STL
///
namespace std
{
size_t hash<nd::node_ref>::operator()(const nd::node_ref& node_reference) const
{
	return hash<std::string>()(node_reference.name);
}
size_t hash<nd::graph_ref>::operator()(const nd::graph_ref& graph_reference) const
{
	return hash<std::string>()(graph_reference.name);
}

ostream& operator<<(ostream& os, const nd::node_ref& node_reference)
{
	os << nd::json(node_reference);
	return os;
}
ostream& operator<<(ostream& os, const nd::graph_ref& graph_reference)
{
	os << nd::json(graph_reference);
	return os;
}
}; // namespace std

/// 
/// Operators
/// 
namespace nd
{
bool node_ref::operator==(const node_ref& other) const { return this->name == other.name; }
bool node_ref::operator!=(const node_ref& other) const { return !(*this == other); }
bool graph_ref::operator==(const graph_ref& other) const { return this->name == other.name; }
bool graph_ref::operator!=(const graph_ref& other) const { return !(*this == other); }
}; // namespace nd

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
using namespace nd;

void adl_serializer<node_ref>::to_json(nd::json& j, const node_ref& node_ref) { j = node_ref.name; }
void adl_serializer<node_ref>::from_json(const nd::json& j, node_ref& node_ref) { node_ref.name = j; }

void adl_serializer<graph_ref>::to_json(nd::json& j, const nd::graph_ref& graph_ref) { j = graph_ref.name; }
void adl_serializer<graph_ref>::from_json(const nd::json& j, nd::graph_ref& graph_ref) { graph_ref.name = j; }
}; // namespace nlohmann