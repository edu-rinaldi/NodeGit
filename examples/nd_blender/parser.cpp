#include "parser.h"

#include <fmt/format.h>
#include <nodediff/diff.h>
#include <nodediff/utility/utility.h>
#include <nodediff/utility/uuid.h>

namespace nd
{
namespace blender
{
	static node parse_blender_node(const nd::json& bl_node,
								   const std::unordered_map<std::string, std::string>& graph_name_uuid,
								   const std::unordered_map<int, std::string>& node_idx_uuid)
	{
		nd::node node;
		const std::string& bl_node_type = bl_node.at(nkit::NODE_NAME);
		// Parse: "node_name", "x", "y", "node_width", "node_height", "node_width_hidden"
		add_property_value(node, NODE_TYPE, bl_node_type);
		add_property_value(node, NODE_X, bl_node.at(nkit::NODE_X));
		add_property_value(node, NODE_Y, bl_node.at(nkit::NODE_Y));
		add_property_value(node, NODE_WIDTH, bl_node.at(nkit::NODE_WIDTH));
		add_property_value(node, NODE_HEIGHT, bl_node.at(nkit::NODE_HEIGHT));
		add_property_value(node, NODE_WIDTH_HIDDEN, bl_node.at(nkit::NODE_WIDTH_HIDDEN));

		// Parse: "parent"
		node_ref parent_id = node_ref::invalid_ref;

		const nd::json& bl_parent_idx = bl_node.at(nkit::NODE_PARENT);
		if (bl_parent_idx.is_number()) { parent_id.name = node_idx_uuid.at(bl_parent_idx.get<int>()); }
		add_node_reference(node, NODE_PARENT, parent_id);

		// Parse: "node_tree"
		nd::graph_ref graph_ref = graph_ref::invalid_ref;

		if (bl_node.contains(nkit::NODETREE))
		{
			const nd::json& node_tree	  = bl_node.at(nkit::NODETREE);
			const std::string& graph_name = node_tree.at(nkit::NODETREE_NAME);
			graph_ref.name				  = graph_name_uuid.at(graph_name);
			add_property_value(node, "p.group_name", value(graph_name));
		}
		add_graph_reference(node, NODE_NODEGROUP, graph_ref);

		// Parse: "attrs"
		for (const nd::json& bl_attribute : bl_node.at(nkit::NODE_ATTRIBUTES))
		{
			const std::string& attribute_name = bl_attribute.at(nkit::NODE_ATTRIBUTE_NAME);

			if (attribute_name == "name") { continue; }

			// If attribute is an image => add texture reference
			if (attribute_name == "image") { add_texture_reference(node, "a.image", bl_attribute); }
			else
			{
				add_property_value(node, fmt::format("{}{}", NODE_ATTRIBUTE_PREFIX, attribute_name),
								   bl_attribute.at(nkit::NODE_ATTRIBUTE_VALUE));
			}
		}

		// Parse: "inputs"
		for (const auto& [socket_idx, bl_socket] : enumerate(bl_node.at(nkit::NODE_INPUTS)))
		{
			const std::string& bl_socket_name = bl_socket.at(nkit::NODE_SOCKET_NAME);
			std::string socket_id			  = fmt::format("i.{}.{}", socket_idx, bl_socket_name);

			value bl_socket_default_value = {};
			// NodeSocketGeometry, NodeSocketInterfaceGeometry, NodeSocketInterfaceShader, NodeSocketShader
			// Above sockets do not have default value
			if (bl_socket.contains(nd::blender::nkit::NODE_SOCKET_VALUE))
			{
				bl_socket_default_value = bl_socket.at(nd::blender::nkit::NODE_SOCKET_VALUE).get<value>();
			}

			// Add socket by adding its value properties and by initializing an empty edge
			// GeometryNodeJoinGeometry needs virtual sockets, i.e. a multiple-value socket
			size_t virtual_idx = 0;
			if (bl_node_type == "GeometryNodeJoinGeometry")
			{
				for (virtual_idx = 0; virtual_idx < 16; ++virtual_idx)
				{
					std::string virtual_socket_id = fmt::format("{}[{}]", socket_id, virtual_idx);
					add_property_value(node, virtual_socket_id, bl_socket_default_value);

					add_input_reference(node, virtual_socket_id, edge{});
				}
			}
			else
			{
				add_property_value(node, socket_id, bl_socket_default_value);

				add_input_reference(node, socket_id, edge{});
			}
		}

		// Parse: "outputs"
		for (const auto& [socket_idx, bl_socket] : enumerate(bl_node.at(nkit::NODE_OUTPUTS)))
		{
			const std::string& bl_socket_name = bl_socket.at(nkit::NODE_SOCKET_NAME);
			std::string socket_id			  = fmt::format("o.{}.{}", socket_idx, bl_socket_name);

			value bl_socket_default_value = {};
			// NodeSocketGeometry, NodeSocketInterfaceGeometry, NodeSocketInterfaceShader, NodeSocketShader
			// Above sockets do not have default value
			if (bl_socket.contains(nkit::NODE_SOCKET_VALUE))
			{
				bl_socket_default_value = bl_socket.at(nkit::NODE_SOCKET_VALUE).get<value>();
			}

			add_property_value(node, socket_id, bl_socket_default_value);
		}

		return node;
	}

	static graph parse_blender_graph(const nd::json& bl_graph,
									 const std::unordered_map<std::string, std::string>& graph_name_uuid)
	{
		nd::graph graph;

		// Collect all blender nodes and assign a uuid to them
		const std::vector<nd::json>& nodes = bl_graph.at(nkit::NODES_LIST);
		std::unordered_map<int, std::string> node_idx_uuid = {};
		for (const auto& [node_idx, bl_node] : enumerate(nodes))
		{
			node_idx_uuid[node_idx] = nd::uuid().string();
		}

		// For each node => parse it and add it to graph
		for (const auto& [node_idx, bl_node] : enumerate(nodes))
		{
			nd::node_ref node_id{.name = node_idx_uuid.at(node_idx)};
			add_node(graph, node_id, parse_blender_node(bl_node, graph_name_uuid, node_idx_uuid));
		}

		// Collect edges
		std::unordered_map<int, std::unordered_map<int, std::vector<nd::json>>> per_node_per_socket_edges = {};
		for (const nd::json& edge : bl_graph.at(nkit::LINKS_LIST))
		{
			int to_node_idx		   = edge.at(nkit::TO_NODE_INDEX);
			int to_socket_idx	   = edge.at(nkit::TO_SOCKET_INDEX);
			auto& per_socket_edges = per_node_per_socket_edges[to_node_idx];
			auto& edges			   = per_socket_edges[to_socket_idx];
			per_node_per_socket_edges.at(to_node_idx).at(to_socket_idx).push_back(edge);
		}
		// For each edge => add it to corresponding nodes
		for (const auto& [to_node_idx, per_socket_edges] : per_node_per_socket_edges)
		{
			for (const auto& [to_socket_idx, edges] : per_socket_edges)
			{
				node_ref to_node_id{.name = node_idx_uuid.at(to_node_idx)};
				nd::node& to_node	   = graph.nodes.at(to_node_id);
				int virtual_socket_idx = 0;
				for (const nd::json& edge : edges)
				{
					const std::string& to_socket_name = edge.at(nkit::TO_SOCKET_NAME);
					const std::string& to_socket_id	  = fmt::format("i.{}.{}", to_socket_idx, to_socket_name);

					int from_node_idx					= edge.at(nkit::FROM_NODE_INDEX);
					const std::string& from_socket_name = edge.at(nkit::FROM_SOCKET_NAME);
					int from_socket_idx					= edge.at(nkit::FROM_SOCKET_INDEX);
					const std::string& from_socket_id	= fmt::format("o.{}.{}", from_socket_idx, from_socket_name);
					node_ref from_node_id{.name = node_idx_uuid.at(from_node_idx)};

					// Handle virtual sockets
					// If first count
					if (edges.size() > 1)
					{
						std::string to_virtual_socket_id = fmt::format("{}[{}]", to_socket_id, virtual_socket_idx);
						set_input_reference(to_node, to_virtual_socket_id,
											nd::edge{.node = from_node_id, .socket_name = from_socket_id});
						++virtual_socket_idx;
					}
					else
					{
						set_input_reference(to_node, to_socket_id,
											nd::edge{.node = from_node_id, .socket_name = from_socket_id});
					}
				}
			}
		}

		// Handle subgraph: save its input interface
		if (bl_graph.contains(nkit::NODETREE_INTERFACE_INPUTS))
		{
			// Create a "virtual" node for storing NodeGroup metadata that should be diffed
			nd::node interface_input_node;
			add_node_reference(interface_input_node, NODE_PARENT, node_ref::invalid_ref);
			add_graph_reference(interface_input_node, NODE_NODEGROUP, graph_ref::invalid_ref);

			const nd::json& bl_interface_inputs = bl_graph.at(nkit::NODETREE_INTERFACE_INPUTS);
			const std::string& bl_graph_name	= bl_graph.at(nkit::NODETREE_NAME);
			int interface_inputs_size			= static_cast<int>(bl_interface_inputs.size());

			add_property_value(interface_input_node, INTERFACE_INPUTS_SIZE, value(interface_inputs_size));
			add_property_value(interface_input_node, NODE_TYPE,
							   value(nkit::NODETREE_INTERFACE_INPUTS));
			add_property_value(interface_input_node, "p.group_name", value(bl_graph_name));
			for (const auto& [idx, interface_inp] : bl_interface_inputs.items())
			{
				add_property_value(interface_input_node, fmt::format("p.{}.default", idx),
								   interface_inp.at(nkit::INTERFACE_INPUTS_DEFAULT).get<value>());
				add_property_value(interface_input_node, fmt::format("p.{}.min", idx),
								   interface_inp.at(nkit::INTERFACE_INPUTS_MIN).get<value>());
				add_property_value(interface_input_node, fmt::format("p.{}.max", idx),
								   interface_inp.at(nkit::INTERFACE_INPUTS_MAX).get<value>());
				add_property_value(interface_input_node, fmt::format("p.{}.hide", idx),
								   interface_inp.at(nkit::INTERFACE_INPUTS_HIDE).get<value>());
			}
			add_node(graph, node_ref{.name = nd::uuid().string()}, interface_input_node);
		}

		return graph;
	}

	static void _collect_graphs(const nd::json& bl_graph, std::unordered_map<std::string, nd::json>& collected_graphs)
	{
		for (const nd::json& bl_node : bl_graph[nkit::NODES_LIST])
		{
			// If this node contains a NodeTree
			if (bl_node.contains(nkit::NODETREE))
			{
				const nd::json& bl_subgraph	  = bl_node.at(nkit::NODETREE);
				const std::string& graph_name = bl_subgraph.at(nkit::NODETREE_NAME);
				// And it has not been collected yet => collect it and collect its subgraphs
				if (!collected_graphs.contains(graph_name))
				{
					collected_graphs[graph_name] = bl_subgraph;
					_collect_graphs(bl_subgraph, collected_graphs);
				}
			}
		}
	}
	
	static std::unordered_map<std::string, nd::json> collect_graphs(const nd::json& bl_script)
	{
		std::unordered_map<std::string, nd::json> collected_graphs = {};

		// Insert starting graph (main)
		collected_graphs["nd_Main"] = bl_script;

		// Collect subgraphs
		_collect_graphs(collected_graphs.at("nd_Main"), collected_graphs);
		return collected_graphs;
	}

	script parse_blender_script(const nd::json& bl_script)
	{
		nd::script script;

		// Collect all blender graphs (i.e.nodegroups) and assign a uuid to them
		std::unordered_map<std::string, nd::json> bl_graphs = collect_graphs(bl_script);
		std::unordered_map<std::string, std::string> graph_name_uuid = {};
		for (const auto& [graph_name, bl_graph] : bl_graphs)
		{
			graph_name_uuid[graph_name] = graph_name == "nd_Main" ? graph_name : nd::uuid().string();
		}

		// For each blender graph => parse it
		for (const auto& [graph_name, bl_graph] : bl_graphs)
		{
			nd::graph_ref graph_id{.name = graph_name_uuid.at(graph_name)};
			add_graph(script, graph_id, parse_blender_graph(bl_graph, graph_name_uuid));
		}
		return script;
	}
}; // namespace blender
}; // namespace nd

namespace nd
{
namespace blender
{
	struct blender_socket
	{
		enum struct socket_type
		{
			invalid,
			input,
			output
		} type = socket_type::invalid;

		int idx				= -1;
		int virtual_idx		= -1;
		std::string name	= "";
		value default_value = {};
	};

	static std::string get_prefix_by_type(const blender_socket::socket_type& socket_type)
	{
		switch (socket_type)
		{
			using enum blender_socket::socket_type;
		case input: return "i";
		case output: return "o";
		case invalid: assert(false && "Invalid blender socket type"); return "";
		}
	}

	static blender_socket::socket_type get_socket_type_by_prefix(char prefix)
	{
		switch (prefix)
		{
			using enum blender_socket::socket_type;
		case 'i': return input;
		case 'o': return output;
		default: return invalid;
		}
	}

	static std::string get_socket_id(const blender_socket& blender_socket)
	{
		assert(blender_socket.type != blender_socket::socket_type::invalid && blender_socket.idx != -1);
		std::stringstream builder;
		builder << get_prefix_by_type(blender_socket.type) << "." << blender_socket.idx << "." << blender_socket.name;
		if (blender_socket.virtual_idx >= 0) { builder << "[" << blender_socket.virtual_idx << "]"; }
		return builder.str();
	}

	static blender_socket build_socket_from_string(const std::string& socket_id)
	{
		blender_socket blender_socket;
		std::string_view sv = socket_id;
		blender_socket.type = get_socket_type_by_prefix(sv.at(0));
		sv.remove_prefix(2);
		size_t dot_idx			  = sv.find(".");
		blender_socket.idx		  = std::stoi(sv.substr(0, dot_idx).data());
		sv						  = sv.substr(dot_idx + 1);
		size_t virtual_socket_idx = sv.find_first_of("[");
		// If there's a virtual socket
		if (virtual_socket_idx != std::string_view::npos)
		{
			blender_socket.name		   = sv.substr(0, virtual_socket_idx);
			sv						   = sv.substr(virtual_socket_idx + 1);
			blender_socket.virtual_idx = std::stoi(sv.substr(0, sv.find_first_of("]")).data());
		}
		else
		{
			blender_socket.name = sv;
		}

		return blender_socket;
	}

	static void node_value_to_preset(const node& node, const preset_rebuild_structure& brs, nd::json& res_node)
	{

		auto& node_attributes = res_node[nkit::NODE_ATTRIBUTES] = nd::json::array();
		auto& node_inputs = res_node[nkit::NODE_INPUTS] = nd::json::array();
		auto& node_outputs = res_node[nkit::NODE_OUTPUTS] = nd::json::array();

		int node_inputs_size = 0, node_outputs_size = 0;
		const std::string& node_type = get_property_value(node, NODE_TYPE).get<std::string>();

		// Virtual socket hack
		bool has_counted_virtual_sockets = false;
		for (const auto& [property_name, value] : node.node_values)
		{
			std::string_view property_view(property_name);
			const char& property_type_char = property_view.at(0);
			property_view.remove_prefix(2);
			switch (property_type_char)
			{
			case 'v':
				// e.g. "x": 1.0
				res_node[property_view.data()] = value;
				break;
			case 'a': { // e.g. "attrs": [{"type_name": "bool", "value": 0, "attr_name": "active_preview"}]
				auto& res_attribute		  = node_attributes.emplace_back();
				std::string brs_node_type = node_type == "ShaderNodeGroup" || node_type == "GeometryNodeGroup"
												? get_property_value(node, "p.group_name").get<std::string>()
												: node_type;
				res_attribute[nkit::NODE_ATTRIBUTE_TYPE] =
					brs.from_node_type.at(brs_node_type).from_attribute_name.at(property_view.data());
				res_attribute[nkit::NODE_ATTRIBUTE_VALUE] = value;
				res_attribute[nkit::NODE_ATTRIBUTE_NAME]  = property_view.data();
			}
			break;
			case 'i':
				if (has_counted_virtual_sockets) { break; }
				if (node_type == "GeometryNodeJoinGeometry") { has_counted_virtual_sockets = true; }
				++node_inputs_size;
				break;
			case 'o': ++node_outputs_size; break;
			case 'p': break;
			default: assert(false && "invalid property name"); break;
			}
		}

		resize_json_array(node_inputs, node_inputs_size);
		resize_json_array(node_outputs, node_outputs_size);

		for (const auto& [property_name, value] : node.node_values)
		{
			std::string_view property_view(property_name);
			const char& property_type_char = property_view.at(0);
			// property_view.remove_prefix(2);
			switch (property_type_char)
			{
			case 'i': { // e.g. "inputs": [{"type_name": "NodeSocketColor", "value": [0,0,0], "name": "Color", "hide":
						// false}]
				blender_socket socket = build_socket_from_string(property_view.data());
				assert(socket.idx >= 0 && socket.idx < node_inputs.size() && "socket idx out of range");
				auto& res_inp = node_inputs[socket.idx];

				std::string brs_node_type = node_type == "ShaderNodeGroup" || node_type == "GeometryNodeGroup"
												? get_property_value(node, "p.group_name").get<std::string>()
												: node_type;

				res_inp[nkit::NODE_SOCKET_TYPE] = brs.from_node_type.at(brs_node_type).from_input_name.at(socket.name);
				if (node_type == "ShaderNodeMapRange" || node_type == "FunctionNodeCompare")
				{
					std::string property_type;
					switch (value.type())
					{
					case value::type::boolean: property_type = "NodeSocketBool"; break;
					case value::type::float_number: property_type = "NodeSocketFloat"; break;
					case value::type::int_number: property_type = "NodeSocketInt"; break;
					case value::type::float_array: {
						size_t array_size = value.get<std::vector<float>>().size();
						if (array_size == 3) { property_type = "NodeSocketVector"; }
						else if (array_size == 4)
						{
							property_type = "NodeSocketColor";
						}
						else
						{
							assert(false && "Invalid array size");
						}
						break;
					}
					case value::type::int_array: {
						size_t array_size = value.get<std::vector<int>>().size();
						if (array_size == 3) { property_type = "NodeSocketVector"; }
						else if (array_size == 4)
						{
							property_type = "NodeSocketColor";
						}
						else
						{
							assert(false && "Invalid array size");
						}
						break;
					}
					case value::type::string: property_type = "NodeSocketString"; break;
					default: assert(false && "Cannot correspond to other types"); break;
					}
					res_inp[nkit::NODE_SOCKET_TYPE] = property_type;
				}

				res_inp[nkit::NODE_SOCKET_VALUE] = value;
				res_inp[nkit::NODE_SOCKET_NAME]	 = socket.name;
				res_inp[nkit::NODE_SOCKET_HIDE]	 = false;
			}
			break;
			case 'o': { // e.g. "outputs": [{"type_name": "NodeSocketColor", "value": [0,0,0], "name": "Color", "hide":
						// false}]
				blender_socket socket = build_socket_from_string(property_view.data());
				assert(socket.idx < node_outputs.size() && "socket idx out of range");
				auto& res_out = node_outputs[socket.idx];

				std::string brs_node_type = node_type == "ShaderNodeGroup" || node_type == "GeometryNodeGroup"
												? get_property_value(node, "p.group_name").get<std::string>()
												: node_type;

				res_out[nkit::NODE_SOCKET_TYPE] = brs.from_node_type.at(brs_node_type).from_output_name.at(socket.name);
				res_out[nkit::NODE_SOCKET_VALUE] = value;
				res_out[nkit::NODE_SOCKET_NAME]	 = socket.name;
				res_out[nkit::NODE_SOCKET_HIDE]	 = false;
			}
			break;
			case 'v':
			case 'a':
			case 'p': break;
			default: assert(false && "invalid property name"); break;
			}
		}
	}

	static void export_nd_script(const script& script, const graph& graph, nd::json& res,
								 const preset_rebuild_structure& brs)
	{
		auto& nodes_list = res[nkit::NODES_LIST] = nd::json::array();
		auto& links_list = res[nkit::LINKS_LIST] = nd::json::array();

		std::unordered_map<node_ref, int> node_id_to_idx;
		for (const auto& [node_id, node] : graph.nodes)
		{

			// Deal with interface inputs: transform node representation into interface_inputs field in json
			if (get_property_value(node, NODE_TYPE) == nkit::NODETREE_INTERFACE_INPUTS)
			{
				auto& interface_inputs = res[nkit::NODETREE_INTERFACE_INPUTS] = nd::json::array();

				// Set NodeGroup name
				res[nkit::NODETREE_NAME] = get_property_value(node, "p.group_name").get<std::string>();

				assert(get_property_value(node, INTERFACE_INPUTS_SIZE).type() == value::type::int_number &&
					   "invalid type, should be int");
				resize_json_array(interface_inputs, get_property_value(node, INTERFACE_INPUTS_SIZE).get<int>());
				for (auto& [idx, group_interface] : interface_inputs.items())
				{
					group_interface[nkit::INTERFACE_INPUTS_DEFAULT] =
						get_property_value(node, fmt::format("p.{}.default", idx));
					group_interface[nkit::INTERFACE_INPUTS_MIN] =
						get_property_value(node, fmt::format("p.{}.min", idx));
					group_interface[nkit::INTERFACE_INPUTS_MAX] =
						get_property_value(node, fmt::format("p.{}.max", idx));
					group_interface[nkit::INTERFACE_INPUTS_HIDE] =
						get_property_value(node, fmt::format("p.{}.hide", idx));
				}
				continue;
			}
			node_id_to_idx[node_id] = nodes_list.size();
			auto& res_node			= nodes_list.emplace_back();
			node_value_to_preset(node, brs, res_node);
		}

		// Deal with references
		for (const auto& [node_id, node] : graph.nodes)
		{
			// If it's interface inputs ==> continue (i.e. already been processed)
			if (get_property_value(node, NODE_TYPE) == nkit::NODETREE_INTERFACE_INPUTS) { continue; }

			auto& res_node = nodes_list[node_id_to_idx.at(node_id)];

			// node references (i.e. parent)
			const node_ref& parent_ref = get_node_reference(node, NODE_PARENT);
			res_node[nkit::NODE_PARENT] =
				parent_ref == node_ref::invalid_ref ? nd::json("None") : nd::json(node_id_to_idx.at(parent_ref));

			// graph references (i.e. NodeGroup)
			const graph_ref& group_graph_ref = get_graph_reference(node, NODE_NODEGROUP);
			if (group_graph_ref != graph_ref::invalid_ref)
			{
				auto& node_tree				   = res_node[nkit::NODETREE];
				node_tree[nkit::NODETREE_NAME] = group_graph_ref.name;
				// Recursive call
				export_nd_script(script, get_graph(script, group_graph_ref), node_tree, brs);
			}

			// TextureReferences -- a.t.m. there's only "a.image"
			if (node.texture_references.contains("a.image"))
			{
				res_node.at(nkit::NODE_ATTRIBUTES).push_back(get_texture_reference(node, "a.image"));
			}

			// Edges
			for (const auto& [socketName, input_ref] : node.input_references)
			{
				if (input_ref == edge::invalid_edge) continue;
				auto& edge = links_list.emplace_back();

				blender_socket from_socket = build_socket_from_string(input_ref.socket_name);
				blender_socket to_socket   = build_socket_from_string(socketName);

				edge[nkit::FROM_NODE_INDEX]	  = node_id_to_idx.at(input_ref.node);
				edge[nkit::FROM_SOCKET_INDEX] = from_socket.idx;
				edge[nkit::FROM_SOCKET_NAME]  = from_socket.name;
				edge[nkit::TO_NODE_INDEX]	  = node_id_to_idx.at(node_id);
				edge[nkit::TO_SOCKET_INDEX]	  = to_socket.idx;
				edge[nkit::TO_SOCKET_NAME]	  = to_socket.name;
			}
		}

		res[nkit::EDITOR_TYPE] = brs.editor_type;
		res[nkit::SHADER_TYPE] = "OBJECT";
	}

	nd::json export_nd_script(const script& script, const preset_rebuild_structure& brs)
	{
		nd::json res;
		const graph& main = get_graph(script, graph_ref{.name = MAIN_GRAPH_ID});
		export_nd_script(script, main, res, brs);
		return res;
	}
}; // namespace blender
}; // namespace nd

namespace nlohmann
{
void adl_serializer<nd::blender::preset_rebuild_structure>::to_json(nd::json& j,
																	const nd::blender::preset_rebuild_structure& value)
{
	j = nd::json::object();
	for (const auto& [node_type, node_rs] : value.from_node_type)
	{
		j[node_type] = node_rs;
	}
}
void adl_serializer<nd::blender::preset_rebuild_structure>::from_json(const nd::json& j,
																	  nd::blender::preset_rebuild_structure& value)
{
	for (const auto& [node_type, node_rs] : j.items())
	{
		value.from_node_type[node_type] = node_rs;
	}
}

void adl_serializer<nd::blender::node_rebuild_structure>::to_json(nd::json& j,
																  const nd::blender::node_rebuild_structure& value)
{
	auto& fromAttrName = j["fromAttributeName"] = nd::json::object();
	for (const auto& [name, type] : value.from_attribute_name)
	{
		fromAttrName[name] = type;
	}

	auto& fromInpName = j["fromInputName"] = nd::json::object();
	for (const auto& [name, type] : value.from_input_name)
	{
		fromInpName[name] = type;
	}

	auto& fromOutName = j["fromOutputName"] = nd::json::object();
	for (const auto& [name, type] : value.from_output_name)
	{
		fromAttrName[name] = type;
	}
}
void adl_serializer<nd::blender::node_rebuild_structure>::from_json(const nd::json& j,
																	nd::blender::node_rebuild_structure& value)
{
	for (const auto& [name, type] : j["fromAttributeName"].items())
	{
		value.from_attribute_name[name] = type;
	}

	for (const auto& [name, type] : j["fromInputName"].items())
	{
		value.from_input_name[name] = type;
	}

	for (const auto& [name, type] : j["fromOutputName"].items())
	{
		value.from_output_name[name] = type;
	}
}
}; // namespace nlohmann