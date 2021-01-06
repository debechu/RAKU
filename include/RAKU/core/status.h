#ifndef RAKU_CORE_ERROR_H
#define RAKU_CORE_ERROR_H

#include <RAKU/export.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum raku_status
{
    RAKU_OK,
    RAKU_NO_MEMORY,
    RAKU_OUT_OF_RANGE,
    
    RAKU_JSON_NO_KEY
};

RAKU_API
const char* raku_status_to_string(enum raku_status status);

#if defined(__cplusplus)
}
#endif

#endif