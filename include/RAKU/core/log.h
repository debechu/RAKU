#ifndef RAKU_CORE_LOG_H
#define RAKU_CORE_LOG_H

#include <RAKU/export.h>

#define LOG_INFO(...)  raku_log(RAKU_LOG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_TRACE(...) raku_log(RAKU_LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_WARN(...)  raku_log(RAKU_LOG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_ERROR(...) raku_log(RAKU_LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) raku_log(RAKU_LOG_LEVEL_FATAL, __VA_ARGS__)

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum LogLevel
{
    RAKU_LOG_LEVEL_INFO,
    RAKU_LOG_LEVEL_TRACE,
    RAKU_LOG_LEVEL_WARN,
    RAKU_LOG_LEVEL_ERROR,
    RAKU_LOG_LEVEL_FATAL
} LogLevel;

RAKU_API
void raku_log(LogLevel level, const char *format, ...);

#if defined(__cplusplus)
}
#endif

#endif