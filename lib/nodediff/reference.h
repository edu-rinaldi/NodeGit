#pragma once
#include "utility/types.h"
#include "value.h"

#include <unordered_map>

// Forward declarations
namespace nd
{
struct node_ref;
struct graph_ref;

struct value;
}; // namespace nd

///
/// STL
///
namespace std
{
/*
 * implementing nd::node_ref hashing
 */
template <>
struct hash<nd::node_ref>
{
	size_t operator()(const nd::node_ref& node_reference) const;
};

/*
 * implementing nd::node_ref hashing
 */
template <>
struct hash<nd::graph_ref>
{
	size_t operator()(const nd::graph_ref& graph_reference) const;
};
} // namespace std

/// 
/// References
///
namespace nd
{
/*
* A reference to a texture.
* Note: At the moment is not treated like a reference, so we just store it as a map of values.
*/
struct texture_ref : std::unordered_map<std::string, value>
{
	using std::unordered_map<std::string, value>::unordered_map;
};

/*
* A reference to a node is modeled as a string. 
* This struct at the moment is just used for enforcing type-checking by the compiler.
*/
struct node_ref
{
	std::string name;
	static const node_ref invalid_ref;

	bool operator==(const node_ref& other) const;
	bool operator!=(const node_ref& other) const;
};

/*
 * A reference to a graph is modeled as a string.
 * This struct at the moment is just used for enforcing type-checking by the compiler.
 */
struct graph_ref
{
	std::string name;
	static const graph_ref invalid_ref;

	bool operator==(const graph_ref& other) const;
	bool operator!=(const graph_ref& other) const;
};

}; // namespace nd

/// 
/// STL
/// 
namespace std
{
ostream& operator<<(ostream& os, const nd::node_ref& node_reference);
ostream& operator<<(ostream& os, const nd::graph_ref& graph_reference);
}; // namespace std

///
/// Serialization/Deserialization with nlohmann::json
///
namespace nlohmann
{
template <>
struct adl_serializer<nd::node_ref>
{
	static void to_json(nd::json& j, const nd::node_ref& node_reference);
	static void from_json(const nd::json& j, nd::node_ref& node_reference);
};

template <>
struct adl_serializer<nd::graph_ref>
{
	static void to_json(nd::json& j, const nd::graph_ref& graph_reference);
	static void from_json(const nd::json& j, nd::graph_ref& graph_reference);
};
}; // namespace nlohmann
