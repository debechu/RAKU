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
        STATUS_CASE(RAKU_JSON_NO_KEY, "(JSON) Key does not exist.")
    }
    return NULL;

#undef STATUS_CASE
}