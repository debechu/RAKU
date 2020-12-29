#ifndef RAKU_EXPORT_H
#define RAKU_EXPORT_H

#if defined(_WIN32) || defined(__CYGWIN__)
    #define RAKU_LOCAL
    #if defined(RAKU_STATIC)
        #define RAKU_API
    #else
        #if defined(__GNUC__)
            #if defined(RAKU_BUILD)
                #define RAKU_API __attribute__((dllexport))
            #else
                #define RAKU_API __attribute__((dllimport))
            #endif
        #elif defined(__clang__) || defined(_MSC_VER)
            #if defined(RAKU_BUILD)
                #define RAKU_API __declspec(dllexport)
            #else
                #define RAKU_API __declspec(dllimport)
            #endif
        #else
            #error "Compiler not supported!"
        #endif
    #endif
#elif defined(__unix__)
    #if defined(RAKU_STATIC)
        #define RAKU_API
        #define RAKU_LOCAL
    #else
        #if defined(__GNUC__) || defined(__clang__)
            #if defined(RAKU_BUILD)
                #define RAKU_API    __attribute__((visibility("default")))
                #define RAKU_LOCAL  __attribute__((visbility("hidden")))
            #else
                #define RAKU_API
                #define RAKU_LOCAL
            #endif
        #else
            #error "Compiler not supported!"
        #endif
    #endif
#else
    #error "Platform not supported!"
#endif

#endif