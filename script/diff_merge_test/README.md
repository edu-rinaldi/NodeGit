# Script: run_single_test.py
Given an ancestor and two versions, this script can be used for diffing the ancestor with the two versions and then merge those two versions into a single one.

## Usage
This script takes 2 positional arguments:
1. `test_path`: path to the test folder containing `Ancestor`, `Version1`, `Version2` and `Merge` folders. (*)
2. `preset_name`: name of the preset to test. (**)

and 3 optional arguments:
1. `--nd-exec`: path to the nd_blender executable (default is `"./bin/Release/nd_blender"`).
2. `--nd-vis`: path to the nd_visualizer script (default is `"./script/nd_visualizer/nd_visualizer.py"`).
3. `--is-shading`: this parameter should be used if preset to test is a shading graph (default is `False`).

(*) A test folder must have the following structure:
* `PresetName/`
    * `Ancestor/bl_ancestor.json`
    * `Version1/bl_version.json`
    * `Version2/bl_version.json`
    * `Merge/`

(**) If `preset_name` is unknown to you, open the NodeKit's Blender preset file; the preset name is given by the key name to which is associated the whole Blender preset.

Example for testing the `"test/diff_merge/Shading/PaintedMetal"`preset (**shading graph**):
```bash
# cwd is NodeGit project root folder
python ./script/diff_merge_test/run_single_test.py "./test/diff_merge/Shading/PaintedMetal/" "PaintedMetal" --is-shading
```

Example for testing the `"test/diff_merge/Geometry/HalloweenSpider"`preset (**geometry graph**):
```bash
# cwd is NodeGit project root folder
python ./script/diff_merge_test/run_single_test.py "test/diff_merge/Geometry/HalloweenSpider" "HalloweenSpider"
```

# Script: run_all_tests.py
This script is used for running `run_single_test.py` on all the test presets in a given folder.

## Usage
This script takes 2 optional arguments:
1. `-s`: path to the directory containing all the *shading* tests.
2. `-g`: path to the directory containing all the *geometry* tests.

Example for testing all the presets in `"test"` folder:
```bash
# cwd is NodeGit project root folder
python ./script/diff_merge_test/run_all_tests.py -s "./test/diff_merge/Shading/" -g "./test/diff_merge/Geometry/"
```

In case you want to exclude a specific test, you can edit the python script and add the name of the folder to exclude into `shading_filter` or `geoemetry_filter` set variables (or you can just delete the preset folder).