#pragma once

#ifdef ND_LOG_ENABLED
#include <iostream>
// Log simple information
#define nd_log(o) std::cout << "[Log] " << o << std::endl
// Log status (shell is colored using green)
#define nd_log_status(status)                                                                                          \
	std::cout << "\033[1;32m"                                                                                          \
			  << "[Status] " << status << "\033[0m" << std::endl
// Log error (shell is colored using red)
#define nd_log_error(error)                                                                                            \
	std::cerr << "\033[1;31m"                                                                                          \
			  << "[" << __FILE__ << ":" << __LINE__ << "] "                                                            \
			  << "[Error] " << error << "\033[0m" << std::endl
// Log warning (shell is colored using yellow)
#define nd_log_warning(warning)                                                                                        \
	std::cout << "\033[1;33m"                                                                                          \
			  << "[" << __FILE__ << ":" << __LINE__ << "] "                                                            \
			  << "[Warning] " << warning << "\033[0m" << std::endl

#else
#define nd_log(o)
#define nd_log_status(status)
#define nd_log_error(error)
#define nd_log_warning(warning)
#endif