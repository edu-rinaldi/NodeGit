#pragma once

// nd::json to std::string
#define json_to_str(json, indent) json.dump(indent)
// Placeholder for highlighting output parameters (i.e. non-const references)
#define out_var