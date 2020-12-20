#ifndef RAKU_DEBUG_H
#define RAKU_DEBUG_H

#include <RAKU/export.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(RAKU_DEBUG)
    #if defined(__cplusplus)
        #include <csignal>
    #else
        #include <signal.h>
        #include <stdbool.h>
    #endif

    #if defined(_MSC_VER)
        #define DEBUG_BREAK() __debugbreak()
    #else
        #define DEBUG_BREAK() raise(SIGTRAP)
    #endif

    #define ASSERT(condition, ...) raku_assert(condition, __VA_ARGS__)
    RAKU_API
    void raku_assert(const bool condition, const char *format, ...);
#else
    #define DEBUG_BREAK()
    #define ASSERT(condition, ...)
#endif

#if defined(__cplusplus)
}
#endif

#endif