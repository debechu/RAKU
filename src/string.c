#include <RAKU/string.h>
#include <RAKU/core/memory.h>
#include <RAKU/debug.h>

#include <limits.h>
#include <string.h>

#define STRING_BASE_CAPACITY 8

RAKU_API
void raku_string_init(struct raku_string *string)
{
    ASSERT(string != NULL,
           "raku_string_init: string must not be NULL!");

    string->chars = NULL;
    string->count = 0;
    string->capacity = 0;
}

RAKU_API
void raku_string_free(struct raku_string *string)
{
    ASSERT(string != NULL,
           "raku_string_free: string must not be NULL!");

    raku_free(string->chars);
    raku_string_init(string);
}

RAKU_API
void raku_string_own(struct raku_string *string, struct raku_string *other)
{
    ASSERT(string != NULL,
           "raku_string_own: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_own: other must not be NULL!");

    raku_string_free(string);
    *string = *other;
    raku_string_init(other);
}

RAKU_API
void raku_string_ownc(struct raku_string *string, char *other)
{
    ASSERT(string != NULL,
           "raku_string_ownc: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_ownc: other must not be NULL!");

    unsigned int count = strnlen(other, UINT_MAX);

    raku_string_free(string);
    string->chars = other;
    string->count = count;
    string->capacity = count;
}

RAKU_API
enum raku_status raku_string_copy(struct raku_string *string, const struct raku_string *other)
{
    ASSERT(string != NULL,
           "raku_string_copy: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_copy: other must not be NULL!");

    raku_string_free(string);
    return raku_string_writes(string, other);
}

RAKU_API
enum raku_status raku_string_copyc(struct raku_string *string, const char *other)
{
    ASSERT(string != NULL,
           "raku_string_copyc: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_copyc: other must not be NULL!");

    raku_string_free(string);
    return raku_string_writesc(string, other);
}

static enum raku_status grow_string(struct raku_string *string, unsigned int size)
{
    ASSERT(string != NULL,
           "grow_string: string must not be NULL!");

    if (size == 0)
        return RAKU_OK;
    else if (size > (UINT_MAX - string->count - 8))
        return RAKU_NO_MEMORY;

    unsigned int new_capacity;
    unsigned int min_capacity = string->count + size;
    if (min_capacity > (UINT_MAX / 2))
        new_capacity = min_capacity+7;
    else
    {
        new_capacity =
            (string->capacity < STRING_BASE_CAPACITY) ?
                STRING_BASE_CAPACITY :
                2 * string->capacity;
        
        if (new_capacity < min_capacity)
            new_capacity = min_capacity;
    }
    
    enum raku_status status = raku_realloc(
        string->chars,
        (new_capacity+1) * sizeof(char),
        (void**)&string->chars
    );

    if (status == RAKU_OK)
        string->capacity = new_capacity;
    
    return status;
}

RAKU_API
enum raku_status raku_string_write(struct raku_string *string, char c)
{
    ASSERT(string != NULL,
           "raku_string_write: string must not be NULL!");

    enum raku_status status = RAKU_OK;
    if (string->count+1 > string->capacity)
    {
        status = grow_string(string, 1);
        if (status != RAKU_OK)
            goto rsw_error;
    }

    string->chars[string->count++] = c;
    string->chars[string->count] = '\0';

rsw_error:
    return status;
}

RAKU_API
enum raku_status raku_string_writes(struct raku_string *string, const struct raku_string *other)
{
    ASSERT(string != NULL,
           "raku_string_writes: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_writes: other must not be NULL!");

    enum raku_status status = RAKU_OK;
    if (string->count+other->count > string->capacity)
    {
        status = grow_string(string, other->count);
        if (status != RAKU_OK)
            goto rsws_error;
    }

    strncpy(string->chars+string->count, other->chars, other->count);
    string->count += other->count;
    string->chars[string->count] = '\0';

rsws_error:
    return status;
}

RAKU_API
enum raku_status raku_string_writesc(struct raku_string *string, const char *other)
{
    ASSERT(string != NULL,
           "raku_string_writesc: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_writesc: other must not be NULL!");

    unsigned int count = strnlen(other, UINT_MAX);
    enum raku_status status = RAKU_OK;
    if (string->count+count > string->capacity)
    {
        status = grow_string(string, count);
        if (status != RAKU_OK)
            goto rswsc_error;
    }

    strncpy(string->chars+string->count, other, count);
    string->count += count;
    string->chars[string->count] = '\0';

rswsc_error:
    return status;
}

RAKU_API
bool raku_string_equal(const struct raku_string *string, const struct raku_string *other)
{
    ASSERT(string != NULL,
           "raku_string_equal: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_equal: other must not be NULL!");

    return
        (string->count == other->count) &&
        (strncmp(string->chars, other->chars, string->count) == 0);
}

RAKU_API
bool raku_string_equalc(const struct raku_string *string, const char *other)
{
    ASSERT(string != NULL,
           "raku_string_equalc: string must not be NULL!");
    ASSERT(other != NULL,
           "raku_string_equalc: other must not be NULL!");

    unsigned int count = strnlen(other, UINT_MAX);
    return
        (string->count == count) &&
        (strncmp(string->chars, other, count) == 0);
}