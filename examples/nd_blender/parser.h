#pragma once
#include <nodediff/utility/types.h>
#include <unordered_map>

// Forward decl.
namespace nd
{
struct script;
struct graph;
struct node;
struct graph_diff;
struct script_diff;

struct graph_ref;

template <typename T>
struct ref_match;
}; // namespace nd

namespace nd
{
namespace blender
{
	namespace nkit
	{
		static const std::string& NODES_LIST  = "nodes_list";
		static const std::string& LINKS_LIST  = "links_list";
		static const std::string& EDITOR_TYPE = "editor_type";
		static const std::string& SHADER_TYPE = "shader_type";
		static const std::string& NODETREE	  = "node_tree";

		static const std::string& NODE_NAME			= "node_name";
		static const std::string& NODE_X			= "x";
		static const std::string& NODE_Y			= "y";
		static const std::string& NODE_WIDTH		= "width";
		static const std::string& NODE_HEIGHT		= "height";
		static const std::string& NODE_PARENT		= "parent";
		static const std::string& NODE_ATTRIBUTES	= "attrs";
		static const std::string& NODE_INPUTS		= "inputs";
		static const std::string& NODE_OUTPUTS		= "outputs";
		static const std::string& NODE_WIDTH_HIDDEN = "width_hidden";

		static const std::string& NODE_ATTRIBUTE_NAME  = "attr_name";
		static const std::string& NODE_ATTRIBUTE_TYPE  = "type_name";
		static const std::string& NODE_ATTRIBUTE_VALUE = "value";

		static const std::string& NODE_SOCKET_HIDE	= "hide";
		static const std::string& NODE_SOCKET_NAME	= "name";
		static const std::string& NODE_SOCKET_TYPE	= "type_name";
		static const std::string& NODE_SOCKET_VALUE = "value";

		static const std::string& NODETREE_NAME				= "name";
		static const std::string& NODETREE_INTERFACE_INPUTS = "interface_inputs";

		static const std::string& INTERFACE_INPUTS_DEFAULT = "default_value";
		static const std::string& INTERFACE_INPUTS_MIN	   = "min_value";
		static const std::string& INTERFACE_INPUTS_MAX	   = "max_value";
		static const std::string& INTERFACE_INPUTS_HIDE	   = "hide_value";

		static const std::string& FROM_NODE_INDEX	= "from_node_index";
		static const std::string& FROM_SOCKET_INDEX = "from_socket_index";
		static const std::string& FROM_SOCKET_NAME	= "from_socket_name";

		static const std::string& TO_NODE_INDEX	  = "to_node_index";
		static const std::string& TO_SOCKET_INDEX = "to_socket_index";
		static const std::string& TO_SOCKET_NAME  = "to_socket_name";
	}; // namespace nkit

	// NodeChange blender representation constants
	static const std::string& MAIN_GRAPH_ID = "nd_Main";

	// Used for storing blender node values (e.g. x/y-position)
	static const std::string& NODE_VALUE_PREFIX = "v.";
	// Used for storing blender "attributes" value (i.e. attributes in "attrs" in blender preset)
	static const std::string& NODE_ATTRIBUTE_PREFIX = "a.";
	// Used for storing input socket related properties
	static const std::string& NODE_INPUT_PREFIX = "i.";
	// Used for storing output socket related properties
	static const std::string& NODE_OUTPUT_PREFIX = "o.";
	// Used for extra node properties that are not blender-default properties
	static const std::string& NODE_PRIVATE_PREFIX = "p.";

	static const std::string& NODE_TYPE			= "v.node_name";
	static const std::string& NODE_PARENT		= "v.parent";
	static const std::string& NODE_WIDTH		= "v.width";
	static const std::string& NODE_HEIGHT		= "v.height";
	static const std::string& NODE_WIDTH_HIDDEN = "v.width_hidden";
	static const std::string& NODE_X			= "v.x";
	static const std::string& NODE_Y			= "v.y";

	static const std::string& NODE_NODEGROUP		= "p.node_group";
	static const std::string& INTERFACE_INPUTS_SIZE = "p.size";

	constexpr const int MAX_NUMBER_VIRTUAL_SOCKETS = 16;
}; // namespace blender
}; // namespace nd

namespace nd
{
namespace blender
{
	struct node_rebuild_structure
	{
		// Map attribute name ==> attribute type
		std::unordered_map<std::string, std::string> from_attribute_name;
		// Map input name ==> input type
		std::unordered_map<std::string, std::string> from_input_name;
		// Map output name ==> output type
		std::unordered_map<std::string, std::string> from_output_name;
	};

	struct preset_rebuild_structure
	{
		std::unordered_map<std::string, node_rebuild_structure> from_node_type;
		// Defaults
		std::string editor_type = "ShaderNodeTree";
		std::string shader_type = "OBJECT";
	};
}; // namespace blender
}; // namespace nd

///
/// Serialization/Deserialization blender graph
///
namespace nd
{

namespace blender
{
	script parse_blender_script(const nd::json& bl_script);
	nd::json export_nd_script(const script& script, const preset_rebuild_structure& brs);
} // namespace blender

} // namespace nd

namespace nlohmann
{
template <>
struct adl_serializer<nd::blender::preset_rebuild_structure>
{
	static void to_json(nd::json& j, const nd::blender::preset_rebuild_structure& value);
	static void from_json(const nd::json& j, nd::blender::preset_rebuild_structure& value);
};

template <>
struct adl_serializer<nd::blender::node_rebuild_structure>
{
	static void to_json(nd::json& j, const nd::blender::node_rebuild_structure& value);
	static void from_json(const nd::json& j, nd::blender::node_rebuild_structure& value);
};
}; // namespace nlohmann