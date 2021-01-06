#ifndef RAKU_STRING_H
#define RAKU_STRING_H

#include <RAKU/core/defs.h>
#include <RAKU/core/status.h>

struct raku_string
{
    char *chars;
    unsigned int count;
    unsigned int capacity;
};

RAKU_API
void raku_string_init(struct raku_string *string);

RAKU_API
void raku_string_free(struct raku_string *string);

RAKU_API
void raku_string_own(struct raku_string *string, struct raku_string *other);

RAKU_API
void raku_string_ownc(struct raku_string *string, char *other);

RAKU_API
enum raku_status raku_string_copy(struct raku_string *string, const struct raku_string *other);

RAKU_API
enum raku_status raku_string_copyc(struct raku_string *string, const char *other);

RAKU_API
enum raku_status raku_string_write(struct raku_string *string, char c);

RAKU_API
enum raku_status raku_string_writes(struct raku_string *string, const struct raku_string *other);

RAKU_API
enum raku_status raku_string_writesc(struct raku_string *string, const char *other);

RAKU_API
bool raku_string_equal(const struct raku_string *string, const struct raku_string *other);

RAKU_API
bool raku_string_equalc(const struct raku_string *string, const char *other);

#endif