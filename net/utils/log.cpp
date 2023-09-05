#include "log.h"


std::once_flag Logger::flag;  
std::shared_ptr<spdlog::logger> Logger::logger_;  