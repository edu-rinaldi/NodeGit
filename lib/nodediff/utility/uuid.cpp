#include "uuid.h"

#include <random>

namespace nd
{
uuid::uuid()
{
	std::random_device rd;
	std::mt19937 engine{rd()};
	std::uniform_int_distribution<int> dist{0, 256}; // Limits of the interval

	for (int index = 0; index < 16; ++index)
	{
		m_data[index] = (unsigned char)dist(engine);
	}

	m_data[6] = ((m_data[6] & 0x0f) | 0x40); // Version 4
	m_data[8] = ((m_data[8] & 0x3f) | 0x80); // Variant is 10
}

std::string uuid::string()
{
	// e.g.: formats to "0065e7d7-418c-4da4-b4d6-b54b6cf7466a"
	char buffer[256] = {0};
	std::snprintf(buffer, 255, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", m_data[0],
				  m_data[1], m_data[2], m_data[3], m_data[4], m_data[5], m_data[6], m_data[7], m_data[8], m_data[9],
				  m_data[10], m_data[11], m_data[12], m_data[13], m_data[14], m_data[15]);
	return buffer;
}
}; // namespace nd
