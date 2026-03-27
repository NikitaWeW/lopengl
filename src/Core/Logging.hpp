#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/fmt/ranges.h"
#include "spdlog/fmt/ostr.h"
#include "glm/gtx/io.hpp"

inline std::shared_ptr<spdlog::logger> sLogger;

#define LOG_FILENAME false

#define LOG_LOGGER(logger, level, ...) logger->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
#define LOG(level, ...) LOG_LOGGER(sLogger, level, __VA_ARGS__)
#define LOG_TRACE(...) LOG(spdlog::level::trace, __VA_ARGS__)
#define LOG_INFO(...) LOG(spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...) LOG(spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG(spdlog::level::err, __VA_ARGS__)

#define LOG_VAR(x) LOG_TRACE("{}: {}", #x, fmt::streamed(x))

inline void initLogger()
{
    #if LOG_FILENAME
    spdlog::set_pattern("%@ %^%v%$");
    #else
    spdlog::set_pattern("%^%v%$");
    #endif
    sLogger = spdlog::stdout_color_mt("sLogger");
    sLogger->set_level(spdlog::level::trace);
}
