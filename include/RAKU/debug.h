#ifndef RAKU_DEBUG_H
#define RAKU_DEBUG_H

#include <RAKU/export.h>
#include <RAKU/core/defs.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(RAKU_DEBUG)
    #if !defined(__has_builtin)
        #define __has_builtin(x) 0
    #endif

    #if defined(_MSC_VER)
        #define DEBUG_BREAK() __debugbreak()
    #elif defined(__clang__) && __has_builtin(__builtin_debugtrap)
        #define DEBUG_BREAK() __builtin_debugtrap()
    #elif defined(__GNUC__)
        #define DEBUG_BREAK() __builtin_trap()
    #else
        #if defined(__cplusplus)
            #include <csignal>
        #else
            #include <signal.h>
        #endif

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