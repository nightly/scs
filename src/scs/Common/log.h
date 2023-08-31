#pragma once

#if (defined _DEBUG)
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#elif (SCS_VERBOSE == 1) 
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#elif (SCS_STATS_OUTPUT == 1)
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#elif (SCS_MINIMAL_STATS == 1)
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#else 
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#endif 

// Log level must be defined prior to including spdlog.h
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/ranges.h>
#include <spdlog/fmt/bundled/color.h>

#if (defined _DEBUG || SCS_VERBOSE == 1)
	#define SCS_TRACE(...) SPDLOG_TRACE( __VA_ARGS__)
	#define SCS_DEBUG(...) SPDLOG_DEBUG( __VA_ARGS__)
	#define SCS_INFO(...) SPDLOG_INFO( __VA_ARGS__)
	#define SCS_INFOSTATS(...) SPDLOG_INFO( __VA_ARGS__)
	#define SCS_WARN(...) SPDLOG_WARN( __VA_ARGS__)
	#define SCS_ERROR(...) SPDLOG_ERROR( __VA_ARGS__)
	#define SCS_CRITICAL(...) SPDLOG_CRITICAL( __VA_ARGS__)
#else
	#define SCS_DEBUG(...)
	#define SCS_TRACE(...)
	#define SCS_INFO(...)
	#define SCS_INFOSTATS(...)
	#define SCS_WARN(...) SPDLOG_WARN( __VA_ARGS__)
	#define SCS_ERROR(...) SPDLOG_ERROR( __VA_ARGS__)
	#define SCS_CRITICAL(...) SPDLOG_CRITICAL( __VA_ARGS__)
#endif

#if (SCS_STATS_OUTPUT == 1)
	#define SCS_STATS(...) SPDLOG_INFO( __VA_ARGS__)
	#define SCS_INFOSTATS(...) SPDLOG_INFO( __VA_ARGS__)
#else 
	#define SCS_STATS(...)
#endif

#if (SCS_MINIMAL_STATS == 1) 
	#define SCS_MINSTATS(...) SPDLOG_INFO( __VA_ARGS__)
	#define SCS_INFOSTATS(...) SPDLOG_INFO( __VA_ARGS__)
#else 
	#define SCS_MINSTATS(...)
#endif


namespace scs {

	inline void LogModeTracing() {
		spdlog::set_level(spdlog::level::trace);
	}

	inline void LogModeDebug() {
		spdlog::set_level(spdlog::level::debug);
	}

}