import argparse
import subprocess
import os


# default nd_blender executable path
ND_BLENDER_EXEC_PATH = "./bin/nd_blender"
# default nd_visualizer python script path
ND_VISUALIZER_PATH = "./script/nd_visualizer/nd_visualizer.py"
# this parameter enable statistics collection (set it to False if statistics are disabled)
ND_STATISTICS_ENABLED = True

def run_parser(blender_preset_name, blender_preset_fp,
            nd_script_fp=None, 
            output_indent_size=None):
    """
    Executes the nd_blender's parser which parses a NodeKit's Blender preset and 
    encodes it in NodeGit's internal model as an nd::script object. 
    The nd::script is then serialized and saved in a json format.

    Parameters:
    - blender_preset_name: name of the NodeKit's Blender preset to parse. If unknown follow (*) procedure for obtaining it.
    - blender_preset_fp: filepath of the NodeKit's Blender preset to parse.
    - nd_script_fp: filepath of the serialied nd::script obtained by running this command. If this parameter 
                    is not set, it will be automatically saved in current working directory as "nd_{blender_preset_name}.json".
    - output_indent_size: indentation size used for output file. If this parameter is not set, it's 4 by default.

    (*) Open the NodeKit's Blender preset file; the preset name is given by the key name to which is associated the whole Blender preset.
    """
    command = [ND_BLENDER_EXEC_PATH, "parse", blender_preset_name, blender_preset_fp]
    # use parametrized nd_script filepath
    if nd_script_fp is not None:
        assert type(nd_script_fp) == type(str()) and len(nd_script_fp) > 0, "invalid filepath"
        command += ["-o", nd_script_fp]
    
    if output_indent_size is not None:
        assert type(output_indent_size) == type(1) and output_indent_size > 0, "invalid indent size"
        command += ["-i", str(output_indent_size)]
    
    completed = subprocess.run(command)
    return completed

def run_exporter(blender_preset_name, nd_script_fp, 
                blender_preset_fp=None, 
                shading_type=None, is_shading_editor=False, 
                output_indent_size=None):
    """
    Executes the nd_blender's exporter which exports a NodeGit's nd::script as a NodeKit's Blender preset file.

    Parameters:
    - blender_preset_name: name of the NodeKit's Blender preset that is obtained after running this command.
    - nd_script_fp: filepath of the file containing the serialized nd::script object to export.
    - blender_preset_fp: filepath of the NodeKit's Blender preset obtained after running this command. If this 
                        parameter is not set, it will be automatically saved in current working directory as "blender_{preset_name}.json".
    - shading_type: shading type field in NodeKit's preset. Choose among the following ["OBJECT" (default), "WORLD", "LINESTYLE"].
    - is_shading_editor: If this flag is set to True the program will export a Shading NodeTree, otherwise a Geometry one. 
                        If this parameter is not set, it's False by default (i.e. a Geoemetry NodeTree is being exported).
    - output_indent_size: indentation size used for output file. If this parameter is not set, it's 4 by default.
    """
    command = [ND_BLENDER_EXEC_PATH, "export", blender_preset_name, nd_script_fp]

    if blender_preset_fp is not None:
        assert type(blender_preset_fp) == type(str()) and len(blender_preset_fp) > 0, "invalid filepath"
        command += ["-o", blender_preset_fp]

    if shading_type is not None:
        assert type(shading_type) == type(str()) and shading_type in {"OBJECT", "WORLD", "LINESTYLE"}, "invalid shading type"
        command += ["--editor-type", shading_type]
    
    if is_shading_editor:
        assert type(is_shading_editor) == type(bool()), "is_shading_editor parameter must be a boolean"
        command += ["--is-shading"]
    
    if output_indent_size is not None:
        assert type(output_indent_size) == type(1) and output_indent_size > 0, "invalid indent size"
        command += ["-i", str(output_indent_size)]
    
    completed = subprocess.run(command)
    return completed

def run_diff(nd_ancestor_fp, nd_version_fp, 
            nd_diff_fp=None, bl_visualization_fp=None, 
            output_indent_size=None, stats_fp=None):
    """
    Executes the nd_blender's differ which diffs two NodeGit's nd::script objects obtaining an nd::script_diff object (then serialized).

    Parameters:
    - nd_ancestor_fp: filepath of the file containing the serialized ancestor nd::script object to diff.
    - nd_version_fp: filepath of the file containing the serialized version nd::script object to diff.
    - nd_diff_fp: filepath of the file containing the serialized nd::script_diff object obtained after running this command. 
                If this parameter is not set, the diff is just printed in console.  
    - bl_visualization_fp: filepath of the file containing the nd::script file prepared to be visualized in Blender (*). 
                            If this parameter is not set, the visualization is not generated. 
    - output_indent_size: indentation size used for output files. If this parameter is not set, it's 4 by default.
    - stats_fp: filepath of the file containing diff statistics (**).

    (*) Note that this is still an nd::script, so before loading it using NodeKit you shall 
    first export it as a NodeKit's Blender preset using the export command.
    (**) Note that for using this option you MUST first enable statistics using CMake option "-DND_STATISTICS_ENABLED". 
    """
    command = [ND_BLENDER_EXEC_PATH, "diff", nd_ancestor_fp, nd_version_fp]

    if nd_diff_fp is not None:
        assert type(nd_diff_fp) == type(str()) and len(nd_diff_fp) > 0, "invalid filepath"
        command += ["-o", nd_diff_fp]

    if bl_visualization_fp is not None:
        assert type(bl_visualization_fp) == type(str()) and len(bl_visualization_fp) > 0, "invalid filepath"
        command += ["-b", bl_visualization_fp]
    
    if output_indent_size is not None:
        assert type(output_indent_size) == type(1) and output_indent_size > 0, "invalid indent size"
        command += ["-i", str(output_indent_size)]

    if stats_fp is not None:
        assert type(stats_fp) == type(str()) and len(stats_fp) > 0, "invalid filepath"
        command += ["-s", stats_fp]
    
    completed = subprocess.run(command)
    return completed

def run_merge(nd_ancestor_fp, nd_version1_fp, nd_version2_fp, 
            nd_merge_fp=None, bl_visualization_fp=None, 
            output_indent_size=None, stats_fp=None):
    """
    Executes the nd_blender's merge comand which merges two versions created starting from same ancestor (the two versions actually are nd::script_diff objects, obtained by using the diff command). The merge result can either be the merged script or a list of conflicts (in case merge fails).

    Parameters:
    - nd_ancestor_fp: filepath of the file containing the serialized ancestor nd::script object.
    - nd_version1_fp: filepath of the file containing the serialized nd::script_diff object representing the diff result obtained by 
                        diffing the ancestor and version 1.
    - nd_version2_fp: filepath of the file containing the serialized nd::script_diff object representing the diff result obtained by 
                        diffing the ancestor and version 2.
    - nd_merge_fp: filepath of the file containing the serialized nd::script_merge_result object obtained after running this command. 
                    If this parameter is not set, the merge result is just printed in console.
    - bl_visualization_fp: filepath of the file containing the nd::script file prepared to be visualized in Blender (*).
                            If this parameter is not set, the visualization is not generated.
    - output_indent_size: indentation size used for output files. If this parameter is not set, it's 4 by default.
    - stats_fp: filepath of the file containing diff statistics (**).

    (*) Note that this is still an nd::script, so before loading it using NodeKit you shall 
    first export it as a NodeKit's Blender preset using the export command.
    (**) Note that for using this option you MUST first enable statistics using CMake option "-DND_STATISTICS_ENABLED". 
    """
    command = [ND_BLENDER_EXEC_PATH, "merge", nd_ancestor_fp, nd_version1_fp, nd_version2_fp]
    if nd_merge_fp is not None:
        assert type(nd_merge_fp) == type(str()) and len(nd_merge_fp) > 0, "invalid filepath"
        command += ["-o", nd_merge_fp]
    
    if bl_visualization_fp is not None:
        assert type(bl_visualization_fp) == type(str()) and len(bl_visualization_fp) > 0, "invalid filepath"
        command += ["-b", bl_visualization_fp]
    
    if output_indent_size is not None:
        assert type(output_indent_size) == type(1) and output_indent_size > 0, "invalid indent size"
        command += ["-i", str(output_indent_size)]
    
    if stats_fp is not None:
        assert type(stats_fp) == type(str()) and len(stats_fp) > 0, "invalid filepath"
        command += ["-s", stats_fp]
    
    completed = subprocess.run(command)
    return completed

def run_nd_visualizer(nd_script_fp, render_fp,
                    nd_diff1_fp=None, nd_diff2_fp=None, 
                    formats=None, 
                    render_as_visual_script=False):
    """
    Executes the nd_visualizer script which is used for rendering NodeGit's internal model.

    Parameters:
    - nd_script_fp: file containing a serialized nd::script object to plot.
    - render_fp: filepath of the render obtained (without extension).
    - nd_diff1_fp: file containing a serialized nd::script_diff object and it is used for highlighting 
                changes from a version. If this parameter is not set, it just not highlight any change. (*)
    - nd_diff2_fp: same as nd_diff1_fp. (*)
    - formats: rendered image format (example: ["png", "svg"]). If this parameter is not set, a png format will be used as default.
    - render_as_visual_script: this parameter enable to render the nd::script using a visual-script-like visualization (advanced visualization). If this parameter is not set, the renderer will use the "simple" visualization.

    (*) If both diff files are provided, the script highlight changes from both diffs. Intuitively, this means that for rendering a merge result (with no conflicts) we can run the nd_visualizer script by passing two diffs.
    """

    command = ["python", ND_VISUALIZER_PATH, nd_script_fp, render_fp]
    if nd_diff1_fp is not None or nd_diff2_fp is not None:
        command += ["--diffs"]

    if nd_diff1_fp is not None:
        assert type(nd_diff1_fp) == type(str()) and len(nd_diff1_fp) > 0, "invalid filepath"
        command += [nd_diff1_fp]
    
    if nd_diff2_fp is not None:
        assert type(nd_diff2_fp) == type(str()) and len(nd_diff2_fp) > 0, "invalid filepath"
        command += [nd_diff2_fp]

    if formats is not None:
        assert type(formats) == type(list()) and len(formats) > 0, "invalid formats list"
        command += ["-f"]
        command += formats
    
    if render_as_visual_script:
        assert type(render_as_visual_script) == type(bool()), "render_as_visual_script parameter must be a boolean"
        command += ["-r", "visual-scripting"]
    
    completed = subprocess.run(command)
    return completed

def run_single_test(test_path, preset_name, is_shading):
    ancestor_path   = os.path.join(test_path, "Ancestor")
    version1_path   = os.path.join(test_path, "Version1")
    version2_path   = os.path.join(test_path, "Version2")
    merge_path      = os.path.join(test_path, "Merge")

    diff1_stats_path = os.path.join(version1_path, "stats.json") if ND_STATISTICS_ENABLED else None
    diff2_stats_path = os.path.join(version2_path, "stats.json") if ND_STATISTICS_ENABLED else None
    merge_stats_path = os.path.join(merge_path, "stats.json") if ND_STATISTICS_ENABLED else None
    
    # parse NodeKit's Blender presets and store them using NodeGit's internal model
    run_parser(preset_name, os.path.join(ancestor_path, "bl_ancestor.json"), os.path.join(ancestor_path, "nd_ancestor.json"))
    run_parser(preset_name, os.path.join(version1_path, "bl_version.json"), os.path.join(version1_path, "nd_version.json"))
    run_parser(preset_name, os.path.join(version2_path, "bl_version.json"), os.path.join(version2_path, "nd_version.json"))

    # diff ancestor - version 1
    run_diff(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version1_path, "nd_version.json"), os.path.join(version1_path, "nd_diff.json"), os.path.join(version1_path, "blv_version.json"), stats_fp=diff1_stats_path)
    # diff ancestor - version 2
    run_diff(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version2_path, "nd_version.json"), os.path.join(version2_path, "nd_diff.json"), os.path.join(version2_path, "blv_version.json"), stats_fp=diff2_stats_path)

    # merge ancestor with version 1 and version 2 diffs
    run_merge(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version1_path, "nd_diff.json"), os.path.join(version2_path, "nd_diff.json"), os.path.join(merge_path, "nd_merge.json"), os.path.join(merge_path, "blv_merge.json"), stats_fp=merge_stats_path)

    # export from NodeGit's format to blender
    run_exporter(preset_name, os.path.join(version1_path, "blv_version.json"), os.path.join(version1_path, "blv_version.json"), is_shading_editor=is_shading)
    run_exporter(preset_name, os.path.join(version2_path, "blv_version.json"), os.path.join(version2_path, "blv_version.json"), is_shading_editor=is_shading)
    run_exporter(preset_name, os.path.join(merge_path, "blv_merge.json"), os.path.join(merge_path, "blv_merge.json"), is_shading_editor=is_shading)

    # visualization using dot

    # ancestor visualization
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(ancestor_path, "ndv_simple"), formats=["png", "svg"])
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(ancestor_path, "ndv_vs"), formats=["png", "svg"], render_as_visual_script=True)

    # diffs visualization
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version1_path, "ndv_simple"), formats=["png", "svg"], nd_diff1_fp=os.path.join(version1_path, "nd_diff.json"))
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version1_path, "ndv_vs"), formats=["png", "svg"], render_as_visual_script=True, nd_diff1_fp=os.path.join(version1_path, "nd_diff.json"))

    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version2_path, "ndv_simple"), formats=["png", "svg"], nd_diff1_fp=os.path.join(version2_path, "nd_diff.json"))
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(version2_path, "ndv_vs"), formats=["png", "svg"], render_as_visual_script=True, nd_diff1_fp=os.path.join(version2_path, "nd_diff.json"))

    # merge visualization
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(merge_path, "ndv_simple"), formats=["png", "svg"], nd_diff1_fp=os.path.join(version1_path, "nd_diff.json"), nd_diff2_fp=os.path.join(version2_path, "nd_diff.json"))
    run_nd_visualizer(os.path.join(ancestor_path, "nd_ancestor.json"), os.path.join(merge_path, "ndv_vs"), formats=["png", "svg"], render_as_visual_script=True, nd_diff1_fp=os.path.join(version1_path, "nd_diff.json"), nd_diff2_fp=os.path.join(version2_path, "nd_diff.json"))


def main():
    global ND_BLENDER_EXEC_PATH, ND_VISUALIZER_PATH
    parser = argparse.ArgumentParser()
    
    parser.add_argument("test_path", type=str, help="Path to the test folder containing Ancestor/Version1/Version2/Merge folders")
    parser.add_argument("preset_name", type=str, help="Name of the preset to test")
    parser.add_argument("--nd-exec", type=str, help="Path to the nd_blender executable", default=None)
    parser.add_argument("--nd-vis", type=str, help="Path to the nd_visualizer script", default=None)
    parser.add_argument("--is-shading", dest="is_shading", action="store_true", help="This parameter should be used if test is for shading graph", default=False)
    
    parsed = parser.parse_args()
    test_path = parsed.test_path
    preset_name = parsed.preset_name
    is_shading = parsed.is_shading
    if parsed.nd_exec is not None:
        ND_BLENDER_EXEC_PATH = parsed.nd_exec
    if parsed.nd_vis is not None:
        ND_VISUALIZER_PATH = parsed.nd_vis

    run_single_test(test_path, preset_name, is_shading)
    

if __name__ == "__main__":
    main()