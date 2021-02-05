#include <RAKU/core/status.h>

#include <stddef.h>

RAKU_API
const char* raku_status_to_string(enum raku_status status)
{
#define STATUS_CASE(status, message) \
    case status: return message;

    switch (status)
    {
        STATUS_CASE(RAKU_OK, "Operation finished successfully.")
        STATUS_CASE(RAKU_NO_MEMORY, "Not enough memory.")
        STATUS_CASE(RAKU_OUT_OF_RANGE, "Out of range.")

        STATUS_CASE(RAKU_JSON_UNEXPECTED_SYMBOL, "(JSON) Unexpected symbol.")
        STATUS_CASE(RAKU_JSON_INVALID_ESCAPE_SEQUENCE, "(JSON) Invalid escape sequence.")
        STATUS_CASE(RAKU_JSON_INVALID_HEX, "(JSON) Invalid hex.")
        STATUS_CASE(RAKU_JSON_INVALID_SURROGATE_PAIR, "(JSON) Invalid surrogate pair.")
        STATUS_CASE(RAKU_JSON_INVALID_CODE_POINT, "(JSON) Invalid code point.")
        STATUS_CASE(RAKU_JSON_UNTERMINATED_STRING, "(JSON) Unterminated string.")
        STATUS_CASE(RAKU_JSON_INVALID_NUMBER, "(JSON) Invalid number.")
        STATUS_CASE(RAKU_JSON_MISSING_PRECISION, "(JSON) Missing floating precision.")
        STATUS_CASE(RAKU_JSON_MISSING_EXPONENT, "(JSON) Missing exponent.")
        STATUS_CASE(RAKU_JSON_EXPECTED_END, "(JSON) Expected end of value.")
    }
    return NULL;

#undef STATUS_CASE
}