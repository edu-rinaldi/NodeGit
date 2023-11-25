#pragma once
#include <functional>
#include <unordered_map>

// Forward declarations
namespace nd
{
struct graph_ref;
struct node_ref;
struct script;
struct node;
struct graph;
}; // namespace nd

namespace nd
{
/*
 * Bidirectional map used for storing matches.
 * RefType is the type of reference we want to store (namely nd::node_ref or nd::graph_ref)
 */
template <typename RefType>
class ref_match
{
  public:
	// Add a new <ancestor, version> match
	void add_match(const RefType& ancestor, const RefType& version)
	{
		m_ancestor_to_version[ancestor] = version;
		m_version_to_ancestor[version]	= ancestor;
	}
	// Remove an existing <ancestor, version> match
	void remove_match(const RefType& ancestor, const RefType& version)
	{
		assert(m_ancestor_to_version.contains(ancestor) && m_version_to_ancestor.contains(version) &&
			   "Trying to remove a non existing <ancestor, version> match");
		m_ancestor_to_version.erase(ancestor);
		m_version_to_ancestor.erase(version);
	}

	// Map the ancestor reference to the matched one in the version
	[[nodiscard]] inline const RefType& to_version(const RefType& ancestor) const
	{
		return m_ancestor_to_version.at(ancestor);
	}
	// Map the version reference to the matched one in the ancestor
	[[nodiscard]] inline const RefType& to_ancestor(const RefType& version) const
	{
		return m_version_to_ancestor.at(version);
	}

	// Given a version reference, it returns true if it has a matched reference in the ancestor
	[[nodiscard]] inline bool has_match_in_ancestor(const RefType& version) const
	{
		return m_version_to_ancestor.contains(version);
	}
	// Given an ancestor reference, it returns true if it has a matched reference in the version
	[[nodiscard]] inline bool has_match_in_version(const RefType& ancestor) const
	{
		return m_ancestor_to_version.contains(ancestor);
	}

  private:
	std::unordered_map<RefType, RefType> m_ancestor_to_version;
	std::unordered_map<RefType, RefType> m_version_to_ancestor;
};

/*
 * Defines an edit cost function as a function that given two references and a bidirectional map of matches,
 * returns a float value representing the edit cost between the two referenced objects.
 */
template <typename RefType>
using cost_fn = std::function<float(const RefType&, const RefType&, const ref_match<RefType>&)>;

/*
 * A match pass is modeled as a structure containing two objects:
 * 1- The edit cost function to use in a given pass
 * 2- The threshold value to use in a given pass
 *
 * Passes are used by the matching algorithm to allow cascading use of multiple matching's heuristics.
 */
template <typename RefType>
struct match_pass
{
	nd::cost_fn<RefType> cost_fn;
	float threshold;
};
}; // namespace nd

///
/// Edit cost functions and matching algorithms
/// 
namespace nd
{
/*
 * Node edit cost function described in NodeGit's paper work.
 * Note: this function is used by the matching algorithm for obtaining a nd::ref_match<node_ref> object, namely
 * a matching between nodes of two graphs.
 *
 * Function parameters:
 *	- ancestor: ancestor node
 *	- version: version node
 *	- graph_matches: bidirectional map of graph matches calculated so far
 *	- node_matches: bidirectional map of node matches calculated so far
 *
 * Returns: the cost required for editing the ancestor node so to be the version node. Edit cost is normalized in the
 *			range [0, 1].
 */
[[nodiscard]] float edit_cost(const node& ancestor, const node& version, const ref_match<graph_ref>& graph_matches,
							  const ref_match<node_ref>& node_matches);
/*
 * Graph edit cost function described in NodeGit's paper work.
 * Note: this function is used by the matching algorithm for obtaining a nd::ref_match<graph_ref> object, namely
 * a matching between graphs of two scripts.
 *
 * Function parameters:
 *	- ancestor: ancestor graph
 *	- version: version graph
 *
 * Returns: the cost required for editing the ancestor graph so to be the version graph. Edit cost is normalized in the
			range [0, 1] (actually it can be >=1, but it can be clamped to 1).
 */
[[nodiscard]] float edit_cost(const graph& ancestor, const graph& version);

/*
 * Matches ancestor and version scripts' graphs using the matching algorithm.
 * It returns the bidirectional map containing all the matched graphs.
 */
[[nodiscard]] ref_match<graph_ref> match_graphs(const script& ancestor, const script& version);
/*
 * Matches ancestor and version graphs' nodes using the matching algorithm.
 * It returns the bidirectional map containing all the matched nodes.
 */
[[nodiscard]] ref_match<node_ref> match_nodes(const graph& ancestor, const graph& version,
											  const ref_match<graph_ref>& graph_matches);
}; // namespace nd