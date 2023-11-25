#pragma once
#include <string>

namespace nd
{
/*
* Each nd::uuid object corresponds to a new generated uuid.
* Took inspiration from: https://github.com/rkg82/uuid-v4/blob/main/uuid/v4/uuid.h
*/
class uuid
{
  public:
	uuid();
	// Returns uuid formatted as string
	[[nodiscard]] std::string string();
  private:
	unsigned char m_data[16] = {0};
};
} // namespace nd