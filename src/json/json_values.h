#ifndef RAKU_JSON_VALUES_H
#define RAKU_JSON_VALUES_H

#include <RAKU/json.h>

typedef uint32_t string_hash;

struct json_value
{
    enum JsonValueType type;
};

struct json_bool
{
    struct json_value _header;
    bool value;
};

struct json_number
{
    struct json_value _header;
    double value;
};

struct json_string
{
    struct json_value _header;
    string_hash hash;
    struct raku_string value;
};

struct json_array
{
    struct json_value _header;
    struct json_value **values;
    unsigned int count;
    unsigned int capacity;
};

struct json_object
{
    struct json_value _header;
    struct json_string *keys;
    struct json_value **values;
    unsigned int count;
    unsigned int capacity;
};

RAKU_LOCAL
void raku_json_bool_init(struct json_bool *boolean);

RAKU_LOCAL
void raku_json_number_init(struct json_number *number);

RAKU_LOCAL
void raku_json_string_init(struct json_string *string);

RAKU_LOCAL
void raku_json_array_init(struct json_array *array);

RAKU_LOCAL
void raku_json_object_init(struct json_object *object);

RAKU_LOCAL
void raku_json_string_free(struct json_string *string);

RAKU_LOCAL
void raku_json_array_free(struct json_array *array);

RAKU_LOCAL
void raku_json_object_free(struct json_object *object);

#endif