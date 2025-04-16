#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <cstring>

// partialy copied from https://github.com/yksz/c-logger because of cmake unconfigurability

#if defined(_WIN32) || defined(_WIN64)
 #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
 #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif /* defined(_WIN32) || defined(_WIN64) */

enum LogLevel
{
    LogLevel_TRACE,
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_WARN,
    LogLevel_ERROR,
    LogLevel_FATAL
};

#ifdef NDEBUG
    #define LOG_TRACE(...)
    #define LOG_DEBUG(...)
#else
    #define LOG_TRACE(fmt, ...) logger_log(LogLevel_TRACE, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_DEBUG(fmt, ...) logger_log(LogLevel_DEBUG, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#endif

#define LOG_INFO(fmt, ...)  logger_log(LogLevel_INFO , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  logger_log(LogLevel_WARN , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger_log(LogLevel_ERROR, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) logger_log(LogLevel_FATAL, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)

inline char const *logger_levelToString(LogLevel level) {
    switch (level)
    {
    case LogLevel_TRACE: return "TRC";
    case LogLevel_DEBUG: return "DBG";
    case LogLevel_INFO : return "INF";
    case LogLevel_WARN : return "WRN";
    case LogLevel_ERROR: return "ERR";
    case LogLevel_FATAL: return "FAT";
    default            : return "UNK";
    }
};
inline void logger_log(LogLevel level, const char* file, int line, const char* fmt, ...) 
{
    printf("%s %15s:%i  ", logger_levelToString(level), file, line);
    va_list arglist;
    va_start(arglist, fmt);
    vprintf(fmt, arglist);
    va_end(arglist);
    printf("\n");
}
