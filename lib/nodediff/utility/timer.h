#pragma once
#include <chrono>

namespace nd
{
/*
* Simple scoped timer, this means that timer::start() is invoked in constructor.
*/
class timer
{
  public:
	timer();
	// Start the timer
	void start();
	// Stop the timer
	void stop();
	// Returns the amount of milliseconds elapsed since timer has been started
	[[nodiscard]] double milliseconds();
	// Returns the amount of seconds elapsed since timer has been started
	[[nodiscard]] double seconds();

  private:
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	std::chrono::time_point<std::chrono::system_clock> m_end_time;
	bool m_is_running = false;
};
};