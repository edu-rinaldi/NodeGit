# NodeGit: Diffing and Merging Node Graphs
*NodeGit* is a framework that allows users to implement diff and merge versioning primitives for a given visual scripting language.

![teaser](github/teaser.png)

This code is crafted for research and educational purposes. It is not recommended to use it in production environments or large-scale projects. The primary goal is to showcase algorithmic concepts and simplify the reproduction of the paper [[NodeGit: Diffing and Merging Node Graphs](https://doi.org/10.1145/3618343)].

## Repository structure
This repository is composed by:

- `lib`: contains all *NodeGit* implemented libraries; at the moment, it is only implemented the main module "`nodediff`", which provides interface data structures and functions to diff and merge functions.
- `examples`: contains several *NodeGit*'s usage examples; at the moment, the only example provided is `nd_blender`, a tool for diffing and merging *Blender* visual scripts (supporting both shading and geometry NodeTrees) using *NodeGit*'s algorithms.
- `test`: contains several a few NodeGit's test examples for each of which there are: ancestor, version 1, version 2 (both versions created editing the ancestor).
- `scripts`: provides python scripts for testing and processing examples; in particular, `nd_visualizer.py` provides a visualization of NodeGit's visual script model (including diff and merge changes).
- `external`: external libraries needed by *NodeGit*.

## Library
`lib/nodediff` is made of the following files:
- `script.{h|cpp}`: contains *NodeGit*'s internal model data structures for representing a visual script (i.e.: `nd::script`, `nd::graph` and `nd::node`).
- `diff.{h|cpp}`: contains data structures and algorithms for diffing script's data.
- `merge.{h|cpp}`: contains data structures and algorithms for merging scripts.
- `matching.{h|cpp}`: contains the matching algorithm used by diff algorithms.
- `value.{h|cpp}`: implements a variadic-type structure using enums.
- `reference.{h|cpp}`: implements `nd::node_ref`, `nd::graph_ref` and `nd::texture_ref` references.
- `utility`: folder containing utilities like a log system (enabled including header and by defining `ND_LOG_ENABLED`), timer, uuid, statistics collector, and other utility functions.

## How to start

### Clone the project
Since *NodeGit* currently uses git's submodule feature, to clone the repository with all its submodules, we recommend using the following command:
```bash
git clone --recursive https://github.com/edu-rinaldi/NodeGit.git
```

### Build the project
*NodeGit* uses [CMake](https://cmake.org/) as build system, so the minimum steps required for building this project are:

1. `mkdir build`
2. `cd build`
3. `cmake ..`

Or just use CMakeGUI. Two options can be enabled/disabled using CMake:

1. `ND_BUILD_EXAMPLES`: builds *NodeGit*'s examples (e.g.: `nd_blender`)
2. `ND_PARALLELIZE`: allows parallel execution of *NodeGit*'s algorithms, using C++ STL parallel algorithms.

This project is known to be compiled on:

- MSVC 14.32.31326 (Visual Studio 2022)
- GCC 12.1.0

## Citation

If you want to include this code in your work, please cite us as:

```
@article{10.1145/3618343,
  author = {Rinaldi, Eduardo and Sforza, Davide and Pellacini, Fabio},
  title = {NodeGit: Diffing and Merging Node Graphs},
  year = {2023},
  issue_date = {December 2023},
  publisher = {Association for Computing Machinery},
  address = {New York, NY, USA},
  volume = {42},
  number = {6},
  issn = {0730-0301},
  url = {https://doi.org/10.1145/3618343},
  doi = {10.1145/3618343},
  abstract = {The use of version control is pervasive in collaborative software projects. Version control systems are based on two primary operations: diffing two versions to compute the change between them and merging two versions edited concurrently. Recent works provide solutions to diff and merge graphics assets such as images, meshes and scenes. In this work, we present a practical algorithm to diff and merge procedural programs written as node graphs. To obtain more precise diffs, we version the graphs directly rather than their textual representations. Diffing graphs is equivalent to computing the graph edit distance, which is known to be computationally infeasible. Following prior work, we propose an approximate algorithm tailored to our problem domain. We validate the proposed algorithm by applying it both to manual edits and to a large set of randomized modifications of procedural shapes and materials. We compared our method with existing state-of-the-art algorithms, showing that our approach is the only one that reliably detects user edits.},
  journal = {ACM Trans. Graph.},
  month = {dec},
  articleno = {265},
  numpages = {12},
  keywords = {procedural node graphs, version control}
}
```
