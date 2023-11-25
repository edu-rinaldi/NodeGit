import json
import graphviz
from argparse import ArgumentParser

# ========================
#         UTILITY
# ========================
def clamp(f : float, min_ : float = 0, max_ : float = 1):
    return max(min_, min(max_, f))

def rgba(r : float = 0, g : float = 0, b : float = 0, a : float = 0):
    r, g, b, a = clamp(r), clamp(g), clamp(b), clamp(a)
    return '#%02x%02x%02x%02x' % (int(r*255),int(g*255),int(b*255),int(a*255))

# ========================
#         GLOBALS
# ========================
# Version 1 edit colors
ADD_COLOR_1 = rgba(0.32, 0.71, 0.34, 1)
EDIT_COLOR_1 = rgba(0.56, 0.43, 0.85, 1)
DELETE_COLOR_1 = rgba(0.75, 0.37, 0.36, 1)

ADD_COLOR_2 = rgba(1, 0.88, 0.39, 1)
EDIT_COLOR_2 = rgba(0.53, 0.82, 0.97, 1)
DELETE_COLOR_2 = rgba(0.86, 0.45, 0.21, 1)
NONE_OPACITY = 0.2
NONE_COLOR =  rgba(1,1,1,1)

NODE_TYPE = "v.node_name"

NODE_VALS = "node_values"
NODE_REFS = "node_references"
GRAPH_REFS = "graph_references"
TEXTURE_REFS = "texture_references"
INPUT_REFS = "input_references"

CHANGE_OPERATION = "operation"
CHANGE_DIFF = "diff"

NONE = "none"
ADD = "add"
DELETE = "del"
EDIT = "edit"

EDGE_NODE = "node"
EDGE_SOCKET = "socket"

# conflicts
DEL_EDIT = "del_edit"
EDIT_DEL = "edit_del"
EDIT_EDIT = "edit_edit"

_DEBUG = False

def get_graph_change(graph_id: str, script_diff: dict):
    graph_change = script_diff.get(graph_id, {CHANGE_OPERATION: NONE, CHANGE_DIFF: dict()})
    return graph_change[CHANGE_OPERATION], graph_change[CHANGE_DIFF]

def get_node_change(node_id: str, graph_diff: dict):
    node_change = graph_diff.get(node_id, {CHANGE_OPERATION: NONE, CHANGE_DIFF: dict()})
    try:
        op, node_diff = node_change[CHANGE_OPERATION], node_change[CHANGE_DIFF]
    except KeyError:
        # HANDLE case in which we have ADD/DELETE of a graph (NodeGit doesn't store "operation" and "diff", but store directly the added/deleted nd::graph)
        op, node_diff = NONE, node_change
    if op == EDIT and node_diff[INPUT_REFS] and not(node_diff[NODE_VALS] or node_diff[NODE_REFS] or node_diff[GRAPH_REFS] or node_diff[TEXTURE_REFS]):
        op, node_diff = NONE, dict()
    return op, node_diff

def get_edge_change(to_node: str, to_socket: str, graph_diff: dict):
    node_change = graph_diff.get(to_node, {CHANGE_OPERATION: NONE, CHANGE_DIFF: dict()})
    try:
        op, node_diff = node_change[CHANGE_OPERATION], node_change[CHANGE_DIFF]
    except KeyError:
        op, node_diff = NONE, node_change
    if op == EDIT and INPUT_REFS in node_diff and to_socket in node_diff[INPUT_REFS]:
        edge_diff = node_diff[INPUT_REFS][to_socket]
    else:
        op = NONE
        edge_diff = dict()
    return op, edge_diff

def mix_colors(color1, color2) -> str:
    return f"{color1};0.5:{color2}"


# ======================================
#           SCRIPT DESCRIPTORS
# ======================================
class UiNode:
    def __init__(self, id : str) -> None:
        self.id : str = id
        self.border_color = rgba(0, 0, 0, 1)
        self.fill_color = rgba(1, 1, 1, 1)
        self.font_color = rgba(0, 0, 0, 1)

        self._node_attr = dict()
    def highlight_changes(self, node_change1: tuple[str, dict], node_change2: tuple[str, dict]):
        pass
    def draw_to_graph(self, gv_graph : graphviz.Digraph) -> None:
        pass
    
class UiSimpleNode(UiNode):
    def __init__(self, id: str, label: str = " ") -> None:
        super().__init__(id)
        self.label = label
    
    def _set_color(self, node_change1: tuple[str, dict], node_change2: tuple[str, dict]):
        op1, node_diff1 = node_change1
        op2, node_diff2 = node_change2

        K_BORDER_COLOR = "border_color"
        K_FILL_COLOR = "fill_color"
        K_FONT_COLOR = "font_color"
        node_color_schema1 = {
            NONE: {K_BORDER_COLOR: rgba(a=NONE_OPACITY), K_FILL_COLOR: rgba(1,1,1, NONE_OPACITY), K_FONT_COLOR: rgba(a=NONE_OPACITY)}, 
            ADD: {K_BORDER_COLOR: rgba(a=1), K_FILL_COLOR: ADD_COLOR_1, K_FONT_COLOR: rgba(a=1)},
            DELETE: {K_BORDER_COLOR: rgba(a=1), K_FILL_COLOR: DELETE_COLOR_1, K_FONT_COLOR: rgba(a=1)},
            EDIT: {K_BORDER_COLOR: rgba(a=1), K_FILL_COLOR: EDIT_COLOR_1, K_FONT_COLOR: rgba(a=1)}
        }
        node_color_schema2 = {
            NONE: {K_BORDER_COLOR: rgba(a=NONE_OPACITY), K_FILL_COLOR: rgba(1,1,1, NONE_OPACITY), K_FONT_COLOR: rgba(a=NONE_OPACITY)}, 
            ADD: {K_BORDER_COLOR: rgba(a=1), K_FILL_COLOR: ADD_COLOR_2, K_FONT_COLOR: rgba(a=1)},
            DELETE: {K_BORDER_COLOR: rgba(a=1), K_FILL_COLOR: DELETE_COLOR_2, K_FONT_COLOR: rgba(a=1)},
            EDIT: {K_BORDER_COLOR: rgba(a=1), K_FILL_COLOR: EDIT_COLOR_2, K_FONT_COLOR: rgba(a=1)}
        }
        c1 = node_color_schema1[op1]
        c2 = node_color_schema2[op2]
        self.border_color = c2[K_BORDER_COLOR] if op2 != NONE else c1[K_BORDER_COLOR]
        self.fill_color = mix_colors(c1[K_FILL_COLOR], c2[K_FILL_COLOR])
        self.font_color = c2[K_FONT_COLOR] if op2 != NONE else c1[K_FONT_COLOR]

    def highlight_changes(self, node_change1: tuple[str, dict], node_change2: tuple[str, dict]):
        self._set_color(node_change1, node_change2)

    def draw_to_graph(self, gv_graph: graphviz.Digraph) -> None:
        gv_graph.node(self.id, label=self.label, fillcolor=self.fill_color, color=self.border_color, fontcolor=self.font_color, **self._node_attr)
    
class UiVisualScriptingNodeSlot:
    def __init__(self, text: str, slot_color: str = rgba(1, 1, 1, 1)) -> None:
        self.text = text
        self.slot_color = slot_color
    
    def highlight_changes(self, node_change1: tuple[str, dict], node_change2: tuple[str, dict]):
        color_schema1 = {NONE: NONE_COLOR, ADD: ADD_COLOR_1, DELETE: DELETE_COLOR_1, EDIT: EDIT_COLOR_1}
        color_schema2 = {NONE: NONE_COLOR, ADD: ADD_COLOR_2, DELETE: DELETE_COLOR_2, EDIT: EDIT_COLOR_2}

        op1, node_diff1 = node_change1
        op2, node_diff2 = node_change2
        if op1 == EDIT and not any((self.text in node_diff1[attr_type]) for attr_type in [NODE_VALS, NODE_REFS, GRAPH_REFS, TEXTURE_REFS]):
            op1 = NONE
        if op2 == EDIT and not any((self.text in node_diff2[attr_type]) for attr_type in [NODE_VALS, NODE_REFS, GRAPH_REFS, TEXTURE_REFS]):
            op2 = NONE
        
        if op1 != NONE and op2 != NONE:
            self.slot_color = mix_colors(color_schema1[op1], color_schema2[op2])
        else:
            self.slot_color = color_schema1[op1]
            if op2 != NONE:
                self.slot_color = color_schema2[op2]

    def build_slot_label(self) -> str:
        pass

class UiVisualScriptingHeader(UiVisualScriptingNodeSlot):
    def __init__(self, text: str, slot_color: str = rgba(1, 1, 1, 1)) -> None:
        super().__init__(text, slot_color)
    
    def highlight_changes(self, node_change1: tuple[str, dict], node_change2: tuple[str, dict]):
        color_schema1 = {NONE: NONE_COLOR, ADD: ADD_COLOR_1, DELETE: DELETE_COLOR_1, EDIT: NONE_COLOR}
        color_schema2 = {NONE: NONE_COLOR, ADD: ADD_COLOR_2, DELETE: DELETE_COLOR_2, EDIT: NONE_COLOR}

        op1, node_diff1 = node_change1
        op2, node_diff2 = node_change2
        if op1 != NONE and op2 != NONE:
            self.slot_color = mix_colors(color_schema1[op1], color_schema2[op2])
        else:
            self.slot_color = color_schema1[op1]
            if op2 != NONE:
                self.slot_color = color_schema2[op2]
        
    def build_slot_label(self) -> str:
        return f'<tr><td border="1" port="header" colspan="3" bgcolor="{self.slot_color}">{self.text}</td></tr>'

class UiVisualScriptingAttribute(UiVisualScriptingNodeSlot):
    def __init__(self, text: str, slot_color: str = rgba(1, 1, 1, 1)) -> None:
        super().__init__(text, slot_color)

    def build_slot_label(self) -> str:
        return f'<tr><td bgcolor="#ffffff">  </td><td bgcolor="{self.slot_color}">{self.text}</td><td bgcolor="#ffffff">  </td></tr>'

class UiVisualScriptingInputSocket(UiVisualScriptingNodeSlot):
    def __init__(self, text: str, socket_name: str, slot_color: str = rgba(1, 1, 1, 1), socket_color: str = rgba(1, 1, 1, 1)) -> None:
        super().__init__(text, slot_color)
        self.socket_name: str = socket_name
        self.socket_color: str = socket_color
    
    def build_slot_label(self) -> str:
        return f'<tr><td bgcolor="{self.socket_color}" port="{self.socket_name}">→</td><td bgcolor="{self.slot_color}">{self.text}</td><td bgcolor="#ffffff">   </td></tr>'

class UiVisualScriptingOutputSocket(UiVisualScriptingNodeSlot):
    def __init__(self, text: str, socket_name: str, slot_color: str = rgba(1, 1, 1, 1), socket_color: str = rgba(1, 1, 1, 1)) -> None:
        super().__init__(text, slot_color)
        self.socket_name: str = socket_name
        self.socket_color: str = socket_color
    
    def build_slot_label(self) -> str:
        return f'<tr><td bgcolor="#ffffff">   </td><td bgcolor="{self.slot_color}">{self.text}</td><td bgcolor="{self.socket_color}" port="{self.socket_name}">→</td></tr>'

class UiVisualScriptingNode(UiNode):
    def __init__(self, id: str) -> None:
        super().__init__(id)
        self.header: UiVisualScriptingHeader = None
        self.attributes: list[UiVisualScriptingInputSocket] = list()
        self.inputs: list[UiVisualScriptingInputSocket] = list()
        self.outputs: list[UiVisualScriptingOutputSocket] = list()

    def highlight_changes(self, node_change1: tuple[str, dict], node_change2: tuple[str, dict]):
        # header
        self.header.highlight_changes(node_change1, node_change2)
        # attributes
        for attribute in self.attributes:
            attribute.highlight_changes(node_change1, node_change2)
        # inputs
        for input in self.inputs:
            input.highlight_changes(node_change1, node_change2)
        # outputs
        for output in self.outputs:
            output.highlight_changes(node_change1, node_change2)

    def build_table(self) -> str:
        gv_label = ['<<table border="0" cellspacing="0" cellborder="1">']
        # header slot
        gv_label.append(self.header.build_slot_label())
        # attributes
        for attribute in self.attributes:
            gv_label.append(attribute.build_slot_label())
        # inputs
        for input_ in self.inputs:
            gv_label.append(input_.build_slot_label())        
        # outputs
        for output in self.outputs:
            gv_label.append(output.build_slot_label())
        gv_label += ['</table>>']
        return "".join(gv_label)

    def draw_to_graph(self, gv_graph: graphviz.Digraph) -> None:
        gv_graph.node(self.id, label=self.build_table(), fillcolor=self.fill_color, color=self.border_color, fontcolor=self.font_color, **self._node_attr)

class UiCommentNode(UiNode):
    def __init__(self, id: str, comment: str) -> None:
        super().__init__(id)
        self.comment = comment
    
    def draw_to_graph(self, gv_graph: graphviz.Digraph) -> None:
        gv_graph.node(self.id, self.comment, shape="note", fixedsize="false", **self._node_attr)

class UiEdgePoint:
    def __init__(self, node: str, socket: str) -> None:
        self.node: str = node
        self.socket: str = socket
    def is_valid(self) -> str:
        return self.node != "" and self.socket != ""
class UiVisualScriptingEdgePoint(UiEdgePoint):
    def __init__(self, node: str, socket: str) -> None:
        super().__init__(node, socket)
    def build_edge_point_label(self) -> str:
        return f"{self.node}:{self.socket}:c"

class UiSimpleEdgePoint(UiEdgePoint):
    def __init__(self, node: str, socket: str) -> None:
        super().__init__(node, socket)
    def build_edge_point_label(self) -> str:
        return self.node

class UiEdge:
    def __init__(self, from_point: UiEdgePoint, to_point: UiEdgePoint) -> None:
        self.from_point: UiEdgePoint = from_point
        self.to_point: UiEdgePoint = to_point
        self.edge_color: str = rgba(0, 0, 0, 1)

        self._edge_attr = dict()

    def is_valid(self) -> bool:
        return self.from_point.is_valid() and self.to_point.is_valid()

    def highlight_changes(self, edge_change1: dict, edge_change2: dict):
        color_schema1 = {NONE: rgba(a=NONE_OPACITY), EDIT: EDIT_COLOR_1}
        color_schema2 = {NONE: rgba(a=NONE_OPACITY), EDIT: EDIT_COLOR_2}
        op1, edge_diff1 = edge_change1
        op2, edge_diff2 = edge_change2
        self.edge_color = color_schema1[op1]
        if op2 != NONE:
            self.edge_color = color_schema2[op2]

    def draw_to_graph(self, gv_graph : graphviz.Digraph) -> None:
        gv_graph.edge(self.from_point.build_edge_point_label(), self.to_point.build_edge_point_label(), color=self.edge_color, **self._edge_attr)
    
class UiGraph:
    def __init__(self, id : str) -> None:
        self.id = id
        self.name = id
        self.nodes : dict[str, UiNode] = dict()
        self.edges : list[UiEdge] = list()
        self.border_color = rgba(0, 0, 0, 1)
        self._graph_attr = dict()
        self._node_attr = dict()
        self._edge_attr = dict()

    def _set_color(self, graph_change1: tuple[str, dict], graph_change2: tuple[str, dict]):
        graph_border_color_schema1 = {NONE: rgba(a=1), ADD: ADD_COLOR_1, DELETE: DELETE_COLOR_1, EDIT: EDIT_COLOR_1 }
        graph_border_color_schema2 = {NONE: rgba(a=1), ADD: ADD_COLOR_2, DELETE: DELETE_COLOR_2, EDIT: EDIT_COLOR_2 }
        op1, graph_diff1 = graph_change1
        op2, graph_diff2 = graph_change2
        color = graph_border_color_schema1[op1]
        if op2 != NONE:
            color = graph_border_color_schema2[op2]
        self.border_color = color

    def highlight_changes(self, graph_change1: tuple[str, dict], graph_change2: tuple[str, dict]):
        # set color for graph
        op1, graph_diff1 = graph_change1
        op2, graph_diff2 = graph_change2
        self._set_color(graph_change1, graph_change2)
        # set color for nodes
        for node_id, node in self.nodes.items():
            node_change1 = get_node_change(node_id, graph_diff1)
            node_change2 = get_node_change(node_id, graph_diff2)
            node.highlight_changes(node_change1, node_change2)

        for edge in self.edges:
            edge_change1 = get_edge_change(edge.to_point.node, edge.to_point.socket, graph_diff1)
            edge_change2 = get_edge_change(edge.to_point.node, edge.to_point.socket, graph_diff2)
            edge.highlight_changes(edge_change1, edge_change2)

    def draw(self) -> graphviz.Digraph:
        gv_graph = graphviz.Digraph(self.name, graph_attr=self._graph_attr, node_attr=self._node_attr, edge_attr=self._edge_attr)
        # gv_graph.attr(label=self.id, color=self.border_color)
        gv_graph.attr(label='', color=self.border_color)
        for node_id, node in self.nodes.items():
            node.draw_to_graph(gv_graph)
        for edge in self.edges:
            edge.draw_to_graph(gv_graph)
        return gv_graph

class UiScript:
    def __init__(self) -> None:
        self.id : str = "script"
        self.graphs : dict[str, UiGraph] = dict()
        self._graph_attr = dict()
        self._node_attr = dict()
        self._edge_attr = dict()

    def highlight_script_diffs(self, script_diff1: dict, script_diff2: dict):
        for graph_id, graph in self.graphs.items():
            graph_change1 = get_graph_change(graph_id, script_diff1)
            graph_change2 = get_graph_change(graph_id, script_diff2)
            graph.highlight_changes(graph_change1, graph_change2)

    def draw(self) -> graphviz.Digraph:
        gv_script = graphviz.Digraph(self.id, graph_attr=self._graph_attr, node_attr=self._node_attr, edge_attr=self._edge_attr)
        for graph_id, graph in self.graphs.items():
            gv_subgraph = graph.draw()
            gv_script.subgraph(gv_subgraph)
        return gv_script

def build_edge_from_nd(node_id: str, nd_socket_name: str, nd_arc: dict[str, str], visual_scripting: bool=False) -> UiEdge:
    if visual_scripting:
        return UiEdge(UiVisualScriptingEdgePoint(nd_arc[EDGE_NODE], nd_arc[EDGE_SOCKET]), UiVisualScriptingEdgePoint(node_id, nd_socket_name))
    else:
        return UiEdge(UiSimpleEdgePoint(nd_arc[EDGE_NODE], nd_arc[EDGE_SOCKET]), UiSimpleEdgePoint(node_id, nd_socket_name))

def build_simple_node_from_nd(node_id: str, nd_node: dict[str, dict]) -> UiSimpleNode:
    return UiSimpleNode(node_id, " ")

def build_vs_node_from_nd(node_id: str, nd_node: dict[str, dict]) -> UiVisualScriptingNode:
    node = UiVisualScriptingNode(node_id)
    node.header = UiVisualScriptingHeader(nd_node[NODE_VALS][NODE_TYPE])
    for attribute_type in [NODE_VALS, NODE_REFS, GRAPH_REFS, TEXTURE_REFS]:
        for attribute_name, attribute in nd_node[attribute_type].items():
            if attribute_name.startswith("i."):
                node.inputs.append(UiVisualScriptingInputSocket(attribute_name, attribute_name))
            elif attribute_name.startswith("o."):
                node.outputs.append(UiVisualScriptingOutputSocket(attribute_name, attribute_name))
            else:
                node.attributes.append(UiVisualScriptingAttribute(attribute_name))
    return node

def build_node_from_nd(node_id: str, nd_node: dict[str, dict], visual_scripting: bool=False) -> UiNode:
    if visual_scripting:
        return build_vs_node_from_nd(node_id, nd_node)
    else:
        return build_simple_node_from_nd(node_id, nd_node)

def build_graph_from_nd(graph_id: str, nd_graph: dict[str, dict], visual_scripting: bool=False) -> UiGraph:
    graph = UiGraph(graph_id)
    for node_id, nd_node in nd_graph.items():
        graph.nodes[node_id] = build_node_from_nd(node_id, nd_node, visual_scripting)
        for nd_socket_name, nd_arc in nd_node[INPUT_REFS].items():
            edge = build_edge_from_nd(node_id, nd_socket_name, nd_arc, visual_scripting)
            if edge.is_valid():
                graph.edges.append(edge)
    return graph

def build_script_from_nd(nd_script: dict[str, dict], visual_scripting: bool=False) -> UiScript:
    script = UiScript()    
    for graph_id, nd_graph in nd_script.items():
        graph = build_graph_from_nd(graph_id, nd_graph, visual_scripting)
        graph.name = f"cluster{graph.name}"
        script.graphs[graph_id] = graph
    return script
# =====================================
#           GRAPHVIZ RENDERER
# =====================================
class Renderer:
    def __init__(self, engine : str = "dot") -> None:
        self.engine = engine
    
    def __render(self, digraph : graphviz.Digraph, filename : str, *formats : str) -> None:
        for format_ in formats:
            digraph.render(format=format_, filename=filename, cleanup=not(_DEBUG))

    def render_script(self, script: UiScript, filename: str, *formats: str) -> None:
        gv_script = script.draw().unflatten(chain=10)
        self.__render(gv_script, filename, *formats)

# Visual scripting-like dot renderer
class VSRenderer(Renderer):
    def __init__(self, engine: str = "dot") -> None:
        super().__init__(engine)
    def render_script(self, script: UiScript, filename: str, *formats: str) -> None:
        script._graph_attr = {"rankdir": "LR", "splines": "true"}
        script._node_attr = {"style":"filled", "margin":"0", "shape":"plaintext"}
        script._edge_attr = {"arrowtail":"dot", "arrowhead":"normal", "dir":"both", "headclip":"false", "tailclip":"false"}
        return super().render_script(script, filename, *formats)

class SimpleRenderer(Renderer):
    def __init__(self, engine: str = "dot") -> None:
        super().__init__(engine)
    def render_script(self, script: UiScript, filename: str, *formats: str) -> None:
        script._graph_attr = {"overlap" : "shrink", "rankdir":"LR", "outputorder":"edgesfirst", "splines":"curved"}
        script._node_attr = {"style":"filled", "shape":"circle", "fixedsize":"true", "width":"0.25pt"}
        script._edge_attr = {"style":"filled", "arrowhead":"normal"}
        return super().render_script(script, filename, *formats)

def apply_diff_node(node : dict[str, dict], diff : dict[str, dict]):
    for property_name, property_value in diff[NODE_VALS].items():
        node[NODE_VALS][property_name] = property_value
    for property_name, node_ref in diff[NODE_REFS].items():
        node[NODE_REFS][property_name] = node_ref
    for property_name, graph_ref in diff[GRAPH_REFS].items():
        node[GRAPH_REFS][property_name] = graph_ref
    for property_name, texture_ref in diff[TEXTURE_REFS].items():
        node[TEXTURE_REFS][property_name] = texture_ref
    for property_name, input_ref in diff[INPUT_REFS].items():
        node[INPUT_REFS][property_name] = input_ref
    return node

def apply_diff_graph(graph : dict[str, dict], diff : dict[str, dict], nodes_coflicts : dict[str, dict] = dict()):
    for node_id, node_change in diff.items():
        if node_id in nodes_coflicts:
            continue
        operation = node_change[CHANGE_OPERATION]
        if operation == ADD:
            graph[node_id] = node_change[CHANGE_DIFF]
        elif operation == DELETE:
            graph[node_id][INPUT_REFS] = dict()
        elif operation == EDIT:
            graph[node_id] = apply_diff_node(graph[node_id], node_change[CHANGE_DIFF])
        else:
            assert False, "Invalid diff operation"
    return graph

def apply_diff_script(script : dict[str, dict], diff : dict[str, dict], graphs_conflicts : dict[str, dict] = dict()):
    for graph_id, graph_change in diff.items():
        nodes_conflicts = dict()
        if graph_id in graphs_conflicts:     
            if graphs_conflicts[graph_id]["type"] != EDIT_EDIT:
                continue
            else:
                nodes_conflicts = graphs_conflicts[graph_id]["nodes"]

        operation = graph_change[CHANGE_OPERATION]
        if operation == ADD:
            script[graph_id] = graph_change[CHANGE_DIFF]
        elif operation == DELETE:
            pass
        elif operation == EDIT:
            script[graph_id] = apply_diff_graph(script[graph_id], graph_change[CHANGE_DIFF], nodes_conflicts)
        else:
            assert False, "Invalid diff operation"
    return script
        

def filter_diff(diff : dict[str, dict]):
    for graph_id, graph_change in diff.items():
        to_delete = []
        if graph_change[CHANGE_OPERATION] != EDIT: continue
        for node_id, node_change in graph_change[CHANGE_DIFF].items():
            node_diff = node_change[CHANGE_DIFF]
            node_vals = node_diff[NODE_VALS]
            # remove v.x and v.y from nodevals
            node_vals.pop("v.x", {})
            node_vals.pop("v.y", {})
            # check if empty diff            
            if not(node_vals or node_diff[NODE_REFS] or node_diff[GRAPH_REFS] or node_diff[TEXTURE_REFS] or node_diff[INPUT_REFS]): 
                to_delete += [node_id]
        
        for node_id in to_delete:
            del graph_change[CHANGE_DIFF][node_id]
    return diff
            

def preprocess_nodes_conflicts(node_conflicts : list) -> dict[str, dict]:
    nd_nodes_conflicts = dict()
    for node_conflict in node_conflicts:
        node_id = node_conflict["node"]
        nd_nodes_conflicts[node_id] = node_conflict
    return nd_nodes_conflicts

def preprocess_graph_conflicts(conflicts : list) -> dict[str, dict]:
    nd_graph_conflicts = dict()
    for graph_conflict in conflicts:
        graph_conflict["nodes"] = preprocess_nodes_conflicts(graph_conflict["nodes"])
        graph_id = graph_conflict["graph"]
        nd_graph_conflicts[graph_id] = graph_conflict
    return nd_graph_conflicts


### MAIN ###
def main() -> None:
    global _DEBUG
    parser = ArgumentParser()
    
    parser.add_argument("input", type=str, help="NodeGit's script to visualize")
    parser.add_argument("output", type=str, help="Output file path (without extension)")
    parser.add_argument('-d', "--diffs", type=str, nargs="+", default=[])
    parser.add_argument('-c', "--conflicts", type=str)
    parser.add_argument('-f', "--formats", type=str, nargs="+", default=["png"])
    parser.add_argument('-D', "--debug", dest="debug", action="store_true")
    parser.add_argument('-r', "--renderer", type=str, choices=["simple", "visual-scripting"], default="simple")
    
    parsed = parser.parse_args()
    nd_script_fp = parsed.input
    output_fp = parsed.output
    formats = parsed.formats
    diffs = parsed.diffs
    conflicts_fp = parsed.conflicts

    _DEBUG = parsed.debug
    is_visual_scripting = parsed.renderer == "visual-scripting"
    with open(nd_script_fp, "r", encoding="utf-8") as f:
        nd_script = json.load(f)
    
    diff1, diff2 = dict(), dict()
    assert len(diffs) <= 2, "Maximum number of diffs is set to 2"
    if len(diffs) >= 1:
        with open(diffs[0], "r", encoding="utf-8") as f:
            diff1 = json.load(f)
    if len(diffs) >= 2:
        with open(diffs[1], "r", encoding="utf-8") as f:
            diff2 = json.load(f)
    
    nd_conflicts = []
    if conflicts_fp:
        with open(conflicts_fp, "r", encoding="utf-8") as f:
            nd_conflicts = json.load(f)    
    nd_conflicts = preprocess_graph_conflicts(nd_conflicts)

    nd_script = apply_diff_script(nd_script, diff1, nd_conflicts)
    nd_script = apply_diff_script(nd_script, diff2, nd_conflicts)
    
    ui_script = build_script_from_nd(nd_script, visual_scripting=is_visual_scripting)
    # hack for removing v.x and v.y from visualization
    diff1 = filter_diff(diff1)
    diff2 = filter_diff(diff2)

    ui_script.highlight_script_diffs(diff1, diff2)
    
    # render
    renderer = VSRenderer() if is_visual_scripting else SimpleRenderer()
    renderer.render_script(ui_script, output_fp, *formats)

if __name__ == "__main__":
    main()