#pragma once
#include "types.h"

namespace nd
{
/*
 * Singleton object that can be used for storing statistics about anything.
 * Since it's a singleton it can be called anywhere in the code, so that no code changes are needed in order to use it.
 */
class statistics_collector
{
  public:
	nd::json json;

  public:
	statistics_collector(const statistics_collector& other) = delete;
	statistics_collector(statistics_collector&& other)		= delete;
	// Obtain singleton instance
	static statistics_collector& instance()
	{
		static statistics_collector s_instance;
		return s_instance;
	}

  private:
	statistics_collector() = default;
};

}; // namespace nd