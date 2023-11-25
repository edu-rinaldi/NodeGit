#include "utility.h"

#include <fstream>

namespace nd
{
bool load_json(const std::string& fp, nd::json& json)
{
	std::ifstream ifs(fp);
	if (!ifs) return false;
	json = json::parse(ifs);
	return !ifs.bad();
}

bool save_json(const nd::json& json, const std::string& fp, int indent)
{
	std::ofstream ofs(fp);
	if (!ofs) return false;
	ofs << json.dump(indent);
	return !ofs.bad();
}
} // namespace nd