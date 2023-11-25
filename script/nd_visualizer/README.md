# Script: nd_visualizer.py
This script is used as visualization tool for NodeGit's models and it's based on graphviz (check requirements). It provides two types of visualization:

1. *simple*: useful for showing the graphs structure
2. *visual-scripting*: useful for showing in a precise manner the graphs structure

## Usage
This script takes two positional arguments:
1. `input`: filepath of the serialied nd::script object to visualize
2. `output`: filepath (without extension) of the output image obtained by running this script.

and 5 optional arguments:
1. `-d` or `--diffs`: list of paths to files containing serialized `nd::script_diff` objects. These are used for highlighting changes made by a version. If not set, the script will just render the `nd::script` object received as input.
2. `-c` or `--conflicts`: path to conflict file (at the moment not used).
3. `-f` or `--formats`: file format of the output image; if not set, `"png"` is used by default.
4. `-D` or `--debug`: help to debug the visualization; if set, the script will also return the *dot* file generated.
5. `-r` or `--renderer`: specify the type of renderer to use (i.e. choose between simple and visual-scripting visualization). Two options: `"simple"` (default) or `"visual-scripting"`. If not set, `"simple"` is used by default.

## Examples
Render a `nd::script` using visual-scripting visualization and output it as png:
```bash
# cwd is NodeGit project root folder
python ./script/nd_visualizer/nd_visualizer.py ./test/diff_merge/PhoneScreen/Ancestor/nd_ancestor.json ./test/diff_merge/PhoneScreen/Ancestor/ndv_vs --renderer "visual-scripting"
# output ./test/diff_merge/PhoneScreen/Ancestor/ndv_vs.png file
``` 

Render a `nd::script`, highlighting changes made by a version, using simple visualization and output it as png and svg:
```bash
# cwd is NodeGit project root folder
python ./script/nd_visualizer/nd_visualizer.py ./test/diff_merge/PhoneScreen/Ancestor/nd_ancestor.json ./test/diff_merge/PhoneScreen/Ancestor/ndv_simple --diffs ./test/diff_merge/PhoneScreen/Version2/nd_diff.json -f png svg
# output ./test/diff_merge/PhoneScreen/Ancestor/ndv_simple.png and ./test/diff_merge/PhoneScreen/Ancestor/ndv_simple.svg files
``` 

Render a `nd::script`, hightlighting changes made by two versions (i.e. a merge), using simple visualization, retrieving also debug informations:
```bash
# cwd is NodeGit project root folder
python ./script/nd_visualizer/nd_visualizer.py ./test/diff_merge/PhoneScreen/Ancestor/nd_ancestor.json ./test/diff_merge/PhoneScreen/Ancestor/ndv_simple --diffs ./test/diff_merge/PhoneScreen/Version1/nd_diff.json ./test/diff_merge/PhoneScreen/Version2/nd_diff.json --debug
# output ./test/diff_merge/PhoneScreen/Ancestor/ndv_simple.png and ./test/diff_merge/PhoneScreen/Ancestor/ndv_simple files
``` 