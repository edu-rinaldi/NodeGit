#include "matching.h"

#include "diff.h"

#if defined(ND_PARALLELIZE)
#include <execution>
#endif

#ifdef ND_STATISTICS_ENABLED
#include "utility/statistic.h"
#include "utility/timer.h"
#endif

///
/// Edit cost functions and matching algorithms
///
namespace nd
{
// Used for infinite cost
constexpr float float_inf = std::numeric_limits<float>::max();

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
float edit_cost(const node& ancestor, const node& version, const ref_match<graph_ref>& graph_matches,
				const ref_match<node_ref>& node_matches)
{
	// If different type ==> max cost
	if (get_node_type(ancestor) != get_node_type(version)) { return nd::float_inf; }

	// Otherwise the cost is given by the number of properties changed
	float changed_properties = 0;

	// Number of property value changed
	changed_properties += diff_node_values(ancestor.node_values, version.node_values);

	// Number of node reference changed
	changed_properties += diff_node_references(ancestor.node_references, version.node_references, node_matches);

	// Number of graph references changed
	changed_properties += diff_graph_references(ancestor.graph_references, version.graph_references, graph_matches);

	// Number of node texture changed
	changed_properties += diff_texture_references(ancestor.texture_references, version.texture_references);

	// Number of input edge changed
	changed_properties += diff_input_references(ancestor.input_references, version.input_references, node_matches);

	// Normalize cost
	int total = ancestor.node_values.size() + ancestor.node_references.size() + ancestor.graph_references.size() +
				ancestor.texture_references.size() + ancestor.input_references.size();
	return changed_properties / static_cast<float>(total);
}

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
float edit_cost(const graph& ancestor, const graph& version)
{
	float cost = 0;

	// Count nodes for each type (ancestor)
	std::unordered_map<std::string, int> ancestor_type_count;
	for (const auto& [node_id, node] : ancestor.nodes)
	{
		const std::string& node_type = get_node_type(node);
		if (ancestor_type_count.contains(node_type)) { ++ancestor_type_count.at(node_type); }
		else
		{
			ancestor_type_count[node_type] = 1;
		}
	}

	// Count nodes for each type (version)
	std::unordered_map<std::string, int> version_type_count;
	for (const auto& [node_id, node] : version.nodes)
	{
		const std::string& node_type = get_node_type(node);
		if (ancestor_type_count.contains(node_type)) { --ancestor_type_count.at(node_type); }
		else if (version_type_count.contains(node_type))
		{
			++version_type_count.at(node_type);
		}
		else
		{
			version_type_count[node_type] = 1;
		}
	}

	// Cost
	for (const auto& [node_type, count] : ancestor_type_count)
	{
		cost += abs(count);
	}
	for (const auto& [node_type, count] : version_type_count)
	{
		cost += abs(count);
	}
	// Normalize cost
	return cost / static_cast<float>(ancestor.nodes.size());
}

/*
 * Matching algorithm described in NodeGit's paper work (+ passes implementation).
 * Given an ancestor and version unordered collection of objects (general implementation), it finds greedly
 * the best match between those two collections' objects.
 *
 * Template parameters:
 *	- RefType: type of references to match
 *	- MapContainer: type of the unordered collection container (it shall be a Map-like container, i.e. it shall store
					key-value pairs)
 *
 * Function parameters:
 *	- ancestor: unordered collection of ancestor <id, object> pairs
 *	- version: unordered collection of version <id, object> pairs
 *	- match_passes: vector of passes to use (note: this vector must have size >= 1)
 *
 * Returns: bidirectional map containing matched objects ids.
 */
template <typename RefType, typename MapContainer, typename = std::enable_if_t<nd::is_mapping_v<MapContainer>>>
ref_match<RefType> match_objects(const MapContainer& ancestor, const MapContainer& version,
								 const std::vector<match_pass<RefType>>& match_passes)
{
#ifdef ND_STATISTICS_ENABLED
	nd::json match_statistics;
	match_statistics["ancestor_size"] = ancestor.size();
	match_statistics["version_size"]  = version.size();
	match_statistics["match_type"]	  = typeid(RefType).name();
	int matched						  = 0;
	float total_match_cost			  = ancestor.size() + version.size();

	std::vector<float> step_total_match_cost;
	step_total_match_cost.reserve(std::min(ancestor.size(), version.size()));

	nd::timer timer;
#endif
	// Init. empty match map
	ref_match<RefType> match = {};
	// Add match between invalid references (because they're the same in all versions)
	match.add_match(RefType::invalid_ref, RefType::invalid_ref);

	// Set containing all the ancestor objects to match
	std::unordered_set<RefType> ancestor_to_match = {};
	ancestor_to_match.reserve(ancestor.size());
	for (const auto& [object_id, object] : ancestor)
	{
		ancestor_to_match.emplace(object_id);
	}

	// Set containing all the version objects to match
	std::unordered_set<RefType> version_to_match;
	version_to_match.reserve(version.size());
	for (const auto& [object_id, object] : version)
	{
		version_to_match.emplace(object_id);
	}

	// Extract first match pass
	assert(match_passes.size() > 0);
	size_t pass_idx			 = 0;
	cost_fn<RefType> cost_fn = match_passes[pass_idx].cost_fn;
	float threshold			 = match_passes[pass_idx].threshold;

#if defined(ND_PARALLELIZE)
	std::mutex m;
#endif
	// As long as there are possible matching pairs performs a greedy assignment step
	while (!version_to_match.empty() && !ancestor_to_match.empty())
	{
		// Initialize best match
		float best_match_edit_cost = nd::float_inf;
		std::pair<RefType, RefType> best_match;
		// Perform a minimum edit cost search
#if defined(ND_PARALLELIZE)
		bool found_flag = false;
		// Find less expensive assignment
		std::for_each(std::execution::par_unseq, version_to_match.begin(), version_to_match.end(),
					  [&](const RefType& version_object_id) {
						  for (const RefType& ancestor_object_id : ancestor_to_match)
						  {
							  float cost = cost_fn(ancestor_object_id, version_object_id, match);
							  assert(cost >= 0 && "Negative cost not allowed");
							  {
								  std::lock_guard<std::mutex> l(m);
								  if (found_flag) { return; }
								  if (cost <= best_match_edit_cost)
								  {
									  best_match		   = {version_object_id, ancestor_object_id};
									  best_match_edit_cost = cost;
								  }
								  // Cost zero is minimum, can't find better
								  if (cost == 0) { found_flag = true; }
							  }
						  }
					  });
#else
		for (const RefType& version_object_id : version_to_match)
		{
			for (const RefType& ancestor_object_id : ancestor_to_match)
			{
				float cost = cost_fn(ancestor_object_id, version_object_id, match);
				assert(cost >= 0 && "Negative cost not allowed");
				if (cost <= best_match_edit_cost)
				{
					best_match			 = {version_object_id, ancestor_object_id};
					best_match_edit_cost = cost;
				}
				// Cost zero is minimum, can't find better
				if (cost == 0) { goto check_match; }
			}
		}
	check_match:
#endif
		// If it's an assignment below threshold ==> add match to match map and remove matches from sets
		if (best_match_edit_cost < threshold)
		{
			match.add_match(best_match.second, best_match.first);
			version_to_match.erase(best_match.first);
			ancestor_to_match.erase(best_match.second);
#ifdef ND_STATISTICS_ENABLED
			// Decrease matching cost
			total_match_cost = (total_match_cost - 2.0) + best_match_edit_cost;
			step_total_match_cost.push_back(total_match_cost);
#endif
		}
		else
		{
			++pass_idx;
			if (pass_idx >= match_passes.size()) { break; }
			cost_fn	  = match_passes[pass_idx].cost_fn;
			threshold = match_passes[pass_idx].threshold;
		}
	}
#ifdef ND_STATISTICS_ENABLED
	timer.stop();
	match_statistics["time"]			 = timer.milliseconds();
	match_statistics["match_map_size"]	 = matched;
	match_statistics["total_match_cost"] = step_total_match_cost;
	nd::statistics_collector::instance().json["matches"].push_back(match_statistics);
#endif
	return match;
}

/*
 * Matching algorithm described in NodeGit's paper work (only one pass is performed).
 * Given an ancestor and version unordered collection of objects (general implementation), it finds greedly
 * the best match between those two collections' objects.
 *
 * Template parameters:
 *	- RefType: type of references to match
 *	- MapContainer: type of the unordered collection container (it shall be a Map-like container, i.e. it shall store
					key-value pairs)
 *
 * Function parameters:
 *	- ancestor: unordered collection of ancestor <id, object> pairs
 *	- version: unordered collection of version <id, object> pairs
 *	- cost_fn: edit cost function to use
 *	- threshold: threshold to use for early stopping
 *
 * Returns: bidirectional map containing matched objects ids.
 */
template <typename RefType, typename MapContainer, typename = std::enable_if_t<nd::is_mapping_v<MapContainer>>>
static ref_match<RefType> match_objects(const MapContainer& ancestor, const MapContainer& version,
										const cost_fn<RefType>& cost_fn, float threshold)
{
	// Create first pass
	std::vector<match_pass<RefType>> passes = {{.cost_fn = cost_fn, .threshold = threshold}};
	// Call matching algorithm
	return match_objects(ancestor, version, passes);
}

/*
 * Graph matching algorithm described in NodeGit's paper work.
 * Given an ancestor and a version scripts, it finds greedly the best match between those scripts' graphs.
 *
 * Function parameters:
 *	- ancestor: ancestor script
 *	- version: version script
 *
 * Returns: bidirectional map containing matched graphs ids.
 */
ref_match<graph_ref> match_graphs(const script& ancestor, const script& version)
{
	// Create graph edit cost function
	auto cost_fn = [&](const graph_ref& ancestor_graph_id, const graph_ref& version_graph_id,
					   const ref_match<graph_ref>& graph_matches) -> float {
		return edit_cost(get_graph(ancestor, ancestor_graph_id), get_graph(version, version_graph_id));
	};
	// Call matching algorithm (single-pass)
	return match_objects<graph_ref>(ancestor.graphs, version.graphs, cost_fn, 0.65f);
}

/*
 * Node matching algorithm described in NodeGit's paper work.
 * Given an ancestor and a version graphs, it finds greedly the best match between those graphs' nodes.
 *
 * Function parameters:
 *	- ancestor: ancestor graph
 *	- version: version graph
 *	- graph_matches: bidirectional map of already matched graphs
 *
 * Returns: bidirectional map containing matched graphs ids.
 */
ref_match<node_ref> match_nodes(const graph& ancestor, const graph& version, const ref_match<graph_ref>& graph_matches)
{
	// Create node edit cost function
	auto cost_fn = [&](const node_ref& ancestor_node_id, const node_ref& version_node_id,
					   const ref_match<node_ref>& node_matches) -> float {
		return edit_cost(get_node(ancestor, ancestor_node_id), get_node(version, version_node_id), graph_matches,
						 node_matches);
	};
	// Call matching algorithm (single-pass)
	return match_objects<node_ref>(ancestor.nodes, version.nodes, cost_fn, 0.35f);
}
}; // namespace nd