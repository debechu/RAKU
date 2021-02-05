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
    
    RAKU_JSON_UNEXPECTED_SYMBOL,
    RAKU_JSON_INVALID_ESCAPE_SEQUENCE,
    RAKU_JSON_INVALID_HEX,
    RAKU_JSON_INVALID_SURROGATE_PAIR,
    RAKU_JSON_INVALID_CODE_POINT,
    RAKU_JSON_UNTERMINATED_STRING,
    RAKU_JSON_INVALID_NUMBER,
    RAKU_JSON_MISSING_PRECISION,
    RAKU_JSON_MISSING_EXPONENT,
    RAKU_JSON_EXPECTED_END
};

RAKU_API
const char* raku_status_to_string(enum raku_status status);

#if defined(__cplusplus)
}
#endif

#endif