#ifndef RAKU_CORE_ERROR_H
#define RAKU_CORE_ERROR_H

#include <RAKU/export.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum RakuStatus
{
    RAKU_OK,
    RAKU_NO_MEMORY,
    RAKU_OUT_OF_RANGE
} RakuStatus;

RAKU_API
const char* raku_status_to_string(RakuStatus status);

#if defined(__cplusplus)
}
#endif

#endif