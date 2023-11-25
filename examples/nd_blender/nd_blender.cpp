#include "parser.h"
#include "utility.h"

#include <argparse/args.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <nodediff/diff.h>
#include <nodediff/merge.h>
#include <nodediff/utility/log.h>
#include <nodediff/utility/timer.h>
#include <nodediff/utility/utility.h>

#ifdef ND_STATISTICS_ENABLED
#include <nodediff/utility/statistic.h>
#endif

static std::string g_brs_fp;

void parse_command(args::Subparser& sp)
{
	using namespace nd;
	args::Positional<std::string> arg_preset_name(sp, "preset_name", "NodeKit Blender preset's name",
												  {args::Options::Required});
	args::Positional<std::string> arg_preset_fp(sp, "preset", "NodeKit Blender preset's file path (json)",
												{args::Options::Required});
	args::ValueFlag<std::string> arg_output_fp(
		sp, "nd_out", "Output file in which to store json serialized NodeDiff script", {'o', "out"});
	args::ValueFlag<size_t> arg_output_indent_size(sp, "indent_size", "Indentation size used for output file",
												   {'i', "indent-size"}, 4);

	// Parse parameters
	sp.Parse();

	// Start a timer
	timer timer;

	// Assign to variables
	const std::string& preset_name = arg_preset_name.Get();
	const std::string& preset_fp   = arg_preset_fp.Get();
	const std::string& output_fp   = arg_output_fp.Get();
	size_t indent_size			   = arg_output_indent_size.Get();

	// Load Blender json preset
	nd::json preset_json;
	if (!load_json(preset_fp, preset_json))
	{
		nd_log_error("Failed to load json at: " << preset_fp);
		return;
	}

	// Parse NodeKit's preset and encode it in NodeDiff's model
	nd::script script = blender::parse_blender_script(preset_json[preset_name]);

	// Save NodeDiff model
	std::string final_output_fp = output_fp.empty() ? fmt::format("nd_{}.json", preset_name) : output_fp;
	if (!save_json(script, final_output_fp, indent_size))
	{
		nd_log_error("Failed to save NodeDiff script at: " << final_output_fp);
		return;
	}
	nd_log_status("Successfully saved NodeDiff script at: " << final_output_fp);
	nd_log_status("Total execution time: " << timer.seconds() << " seconds");
}

void export_command(args::Subparser& sp)
{
	using namespace nd;
	args::Positional<std::string> arg_preset_name(sp, "preset_name", "NodeKit Blender preset's name",
												  {args::Options::Required});
	args::Positional<std::string> arg_nd_preset_fp(
		sp, "nd_preset", "NodeDiff's script preset that shall be exported (json)", {args::Options::Required});
	args::ValueFlag<std::string> arg_output_fp(
		sp, "blender_preset", "Output file in which to store the exported NodeKit's Blender preset (json)",
		{'o', "out"});
	args::ValueFlag<std::string> arg_shading_type(
		sp, "shading_type",
		"Shading type field in NodeKit's preset. Choose among the following [OBJECT (default), WORLD, LINESTYLE]",
		{"editor-type"}, "OBJECT");
	args::Flag arg_is_shading_editor(
		sp, "is_shading_editor", "If this flag is set the program will export a Shading NodeTree, otherwise a Geometry",
		{"is-shading"});
	args::ValueFlag<size_t> arg_output_indent_size(sp, "indent_size", "Indentation size used for output file",
												   {'i', "indent-size"}, 4);

	sp.Parse();

	timer timer;

	// Assign to variables
	const std::string& preset_name	= arg_preset_name.Get();
	const std::string& nd_preset_fp = arg_nd_preset_fp.Get();
	const std::string& output_fp	= arg_output_fp.Get();
	const std::string& shading_type = arg_shading_type.Get();
	bool is_shading_editor			= arg_is_shading_editor.Get();
	size_t indent_size				= arg_output_indent_size.Get();

	// Load NodeDiff preset from json
	nd::script script;
	{
		nd::json tmp;
		if (!load_json(nd_preset_fp, tmp))
		{
			nd_log_error("Failed to load json at: " << nd_preset_fp);
			return;
		}
		script = tmp;
	}
	// Load Blender preset rebuild structure
	nd::blender::preset_rebuild_structure brs;
	{
		nd::json tmp;
		if (!load_json(g_brs_fp, tmp))
		{
			nd_log_error("Failed to load rebuild structure at: " << g_brs_fp);
			return;
		}
		brs = tmp;
	}
	brs.editor_type = is_shading_editor ? "ShaderNodeTree" : "GeometryNodeTree";
	brs.shader_type = shading_type;

	// Transform NodeDiff's script into NodeKit's Blender preset format
	nd::json blender_preset;
	blender_preset[preset_name] = blender::export_nd_script(script, brs);

	// Save it
	std::string final_output_fp = output_fp.empty() ? fmt::format("blender_{}.json", preset_name) : output_fp;
	if (!save_json(blender_preset, final_output_fp, indent_size))
	{
		nd_log_error("Could not save NodeKit's Blender preset at: " << final_output_fp);
		return;
	}
	nd_log_status("Successfully exported script as NodeKit's Blender preset at: " << final_output_fp);
	nd_log_status("Total execution time: " << timer.seconds() << " seconds");
}

void diff_command(args::Subparser& sp)
{
	using namespace nd;
	args::Positional<std::string> arg_preset1(sp, "preset1", "First preset to diff (NodeDiff json)",
											  {args::Options::Required});
	args::Positional<std::string> arg_preset2(sp, "preset2", "Second preset to diff (NodeDiff json)",
											  {args::Options::Required});
	args::ValueFlag<std::string> arg_diff_output(sp, "out_diff", "Output file in which to store the diff ",
												 {'o', "out"});
	args::ValueFlag<std::string> arg_blender_visualization_output(
		sp, "blender_vis", "Output file in which to store blender diff visualization preset", {'b', "blender-vis"});
	args::ValueFlag<size_t> arg_output_indent_size(sp, "indent_size", "Indentation size used for output file",
												   {'i', "indent-size"}, 4);
#ifdef ND_STATISTICS_ENABLED
	args::ValueFlag<std::string> arg_statistics_output(
		sp, "out_statistics", "Output file's path in which to store diff statistics", {'s', "stats"});
#endif
	sp.Parse();

	// Assign to variables
	const std::string& preset1_fp					   = arg_preset1.Get();
	const std::string& preset2_fp					   = arg_preset2.Get();
	const std::string& diff_output_fp				   = arg_diff_output.Get();
	const std::string& blender_visualization_output_fp = arg_blender_visualization_output.Get();
	const size_t& indent_size						   = arg_output_indent_size.Get();
#ifdef ND_STATISTICS_ENABLED
	const std::string& statistics_output_fp = arg_statistics_output.Get();
#endif
	// Load json
	script script1, script2;
	{
		nd::json tmp;
		if (!load_json(preset1_fp, tmp))
		{
			nd_log_error("Failed to load json at: " << preset1_fp);
			return;
		}
		script1 = tmp;
	}
	{
		nd::json tmp;
		if (!load_json(preset2_fp, tmp))
		{
			nd_log_error("Failed to load json at: " << preset2_fp);
			return;
		}
		script2 = tmp;
	}

	// Start a timer
	timer timer;

	// Diff scripts
#ifdef ND_STATISTICS_ENABLED
	auto& statistic			  = nd::statistics_collector::instance();
	statistic.json["matches"] = nd::json::array();
	auto& diff_statistics = statistic.json["diff"] = nd::json::object();
#endif
	script_diff script_diff = diff_scripts(script1, script2, match_graphs(script1, script2));

	// Hack for diff-ignoring node property values
	blender::diff_ignore_node_property_values(script_diff, {"v.x", "v.y", "v.width", "v.height", "v.width_hidden"});

#ifdef ND_STATISTICS_ENABLED
	diff_statistics["time"] = timer.milliseconds();

	if (!statistics_output_fp.empty())
	{
		if (save_json(statistic.json, statistics_output_fp))
		{
			nd_log_status("Statistics saved at: " << statistics_output_fp);
		}
		else
		{
			nd_log_error("Statistics could not be saved at: " << statistics_output_fp);
		}
	}
#endif

	// If diff output specified ==> save it
	if (!diff_output_fp.empty())
	{
		if (save_json(script_diff, diff_output_fp, indent_size)) { nd_log_status("Diff saved at: " << diff_output_fp); }
		else
		{
			nd_log_error("Diff could not be saved at: " << diff_output_fp);
		}
	}
	else
	{
		// Otherwise just print it to console
		std::cout << nd::json(script_diff).dump(indent_size);
	}

	// Diff visualization using Blender preset that can be loaded using NodeKit plugin
	if (!blender_visualization_output_fp.empty())
	{
		nd::apply_diff(script1, script_diff);
		blender::apply_diff_visually(script1, script_diff);
		if (save_json(script1, blender_visualization_output_fp))
		{
			nd_log_status("Blender diff visualization preset file saved at: " << blender_visualization_output_fp);
		}
		else
		{
			nd_log_error(
				"Blender diff visualization preset file could not be saved at: " << blender_visualization_output_fp);
		}
	}

	nd_log_status("Total execution time: " << timer.seconds() << " seconds");
}

void merge_command(args::Subparser& sp)
{
	using namespace nd;
	args::Positional<std::string> arg_ancestor(sp, "ancestor", "Ancestor preset (NodeDiff json)",
											   {args::Options::Required});
	args::Positional<std::string> arg_version1(sp, "diff1", "First version diff (NodeDiff json)",
											   {args::Options::Required});
	args::Positional<std::string> arg_version2(sp, "diff2", "Second version diff (NodeDiff json)",
											   {args::Options::Required});
	args::ValueFlag<std::string> arg_merge_output(
		sp, "merge", "Output file where to store merge result / conflicts (NodeDiff json)", {'o', "out"});
	args::ValueFlag<std::string> arg_blender_visualization_output(
		sp, "blender_vis", "Output file in which to store blender merge visualization preset", {'b', "blender-vis"});
	args::ValueFlag<size_t> arg_indent_size(sp, "indent_size", "Indentation size used for output file",
											{'i', "indent-size"}, 4);
#ifdef ND_STATISTICS_ENABLED
	args::ValueFlag<std::string> arg_statistics_output(
		sp, "out_statistics", "Output file's path in which to store merge statistics", {'s', "stats"});
#endif

	sp.Parse();

	// Assign to variables
	const std::string& ancestor_fp					   = arg_ancestor.Get();
	const std::string& version1_fp					   = arg_version1.Get();
	const std::string& version2_fp					   = arg_version2.Get();
	const std::string& merge_output_fp				   = arg_merge_output.Get();
	const std::string& blender_visualization_output_fp = arg_blender_visualization_output.Get();
	size_t indent_size								   = arg_indent_size.Get();
#ifdef ND_STATISTICS_ENABLED
	const std::string& statistics_output_fp = arg_statistics_output.Get();
#endif

	// Load ancestor
	script ancestor_script;
	{
		nd::json tmp;
		nd_log("Loading ancestor preset...");
		if (!load_json(ancestor_fp, tmp))
		{
			nd_log_error("Failed to load json at: " << ancestor_fp);
			return;
		}
		ancestor_script = tmp;
	}
	script_diff diff1, diff2;
	{
		nd::json tmp;
		nd_log("Loading version 1 diff...");
		if (!load_json(version1_fp, tmp))
		{
			nd_log_error("Failed to load json at: " << version1_fp);
			return;
		}
		diff1 = tmp;
	}
	{
		nd::json tmp;
		nd_log("Loading version 2 diff...");
		if (!load_json(version2_fp, tmp))
		{
			nd_log_error("Failed to load json at: " << version2_fp);
			return;
		}
		diff2 = tmp;
	}

	// Start a timer
	timer timer;

	// Optimize diffs' size
	remove_common_adds(diff1, diff2);

	// Try to merge graphs
	script_merge_result merge_result = merge_scripts(ancestor_script, diff1, diff2);

	bool has_conflicts = merge_has_failed(merge_result);
	const nd::json& merge_or_conflicts =
		has_conflicts ? nd::json(merge_result.conflicts) : nd::json(merge_result.result);
#ifdef ND_STATISTICS_ENABLED
	auto& statistic = nd::statistics_collector::instance();

	statistic.json["time"]			= timer.milliseconds();
	statistic.json["has_conflicts"] = has_conflicts;

	if (!statistics_output_fp.empty())
	{
		if (save_json(statistic.json, statistics_output_fp))
		{
			nd_log_status("Statistics saved at: " << statistics_output_fp);
		}
		else
		{
			nd_log_error("Statistics could not be saved at: " << statistics_output_fp);
		}
	}
#endif
	// If output parameter is set
	if (!merge_output_fp.empty())
	{
		const std::string success_out_msg =
			fmt::format("{} saved at: {}", has_conflicts ? "Conflicts" : "Merge", merge_output_fp);
		const std::string failure_out_msg =
			fmt::format("Failed to save {} at: {}", has_conflicts ? "conflicts" : "merge", merge_output_fp);

		// Save
		if (save_json(merge_or_conflicts, merge_output_fp, indent_size)) { nd_log_status(success_out_msg); }
		else
		{
			nd_log_error(failure_out_msg);
		}
	}
	else
	{
		std::cout << merge_or_conflicts.dump(indent_size);
	}

	if (!blender_visualization_output_fp.empty())
	{
		blender::apply_merge_visually(merge_result.result, diff1, diff2);

		if (save_json(merge_result.result, blender_visualization_output_fp))
		{
			nd_log_status("Blender visualization preset saved at: " << blender_visualization_output_fp);
		}
		else
		{
			nd_log_error("Blender visualization preset could not be saved at: " << blender_visualization_output_fp);
		}
	}

	nd_log_status("Total execution time: " << timer.seconds() << " seconds");
}

int main(int argc, const char** argv)
{
	g_brs_fp = fmt::format("{}/resources/blender_rebuild_structure.json",
						   std::filesystem::path(argv[0]).parent_path().string());
	using namespace nlohmann;
	args::ArgumentParser parser("NodeDiff");

	args::Group commands(parser, "commands");

	args::Command export_(commands, "export", "Export NodeDiff's script to NodeKit's Blender preset", export_command);
	args::Command parse(commands, "parse", "Parse a NodeKit's Blender preset to NodeDiff internal format",
						parse_command);
	args::Command diff(commands, "diff", "Diff two NodeDiff's scripts", diff_command);
	args::Command merge(commands, "merge", "Try to merge two NodeDiff's script diffes given an ancestor",
						merge_command);

	args::HelpFlag help(parser, "help", "Display help menu", {'h', "help"});
	args::CompletionFlag completion(parser, {"complete"});

	try
	{
		parser.ParseCLI(argc, argv);
	}
	catch (const args::Completion& e)
	{
		std::cout << e.what();
		return 0;
	}
	catch (const args::Help&)
	{
		std::cout << parser;
		return 0;
	}
	catch (const args::Error& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	return 0;
}