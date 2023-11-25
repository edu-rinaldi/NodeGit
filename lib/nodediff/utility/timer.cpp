#include "timer.h"

namespace nd
{
timer::timer() { start(); }

void timer::start()
{
	m_start_time = std::chrono::system_clock::now();
	m_is_running = true;
}

void timer::stop()
{
	m_end_time	 = std::chrono::system_clock::now();
	m_is_running = false;
}

double timer::milliseconds()
{
	std::chrono::time_point<std::chrono::system_clock> end_time;

	if (m_is_running) { end_time = std::chrono::system_clock::now(); }
	else
	{
		end_time = m_end_time;
	}

	return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count();
}

double timer::seconds() { return milliseconds() / 1000.0; }
}; // namespace nd