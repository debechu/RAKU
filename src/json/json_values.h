#ifndef RAKU_JSON_VALUES_H
#define RAKU_JSON_VALUES_H

#include <RAKU/json.h>

typedef uint32_t StringHash;

struct JsonValue
{
    enum JsonValueType type;
};

struct JsonBool
{
    struct JsonValue _header;
    bool value;
};

struct JsonNumber
{
    struct JsonValue _header;
    double value;
};

struct JsonString
{
    struct JsonValue _header;
    char *chars;
    StringHash hash;
    unsigned int count;
    unsigned int capacity;
};

struct JsonArray
{
    struct JsonValue _header;
    struct JsonValue **values;
    unsigned int count;
    unsigned int capacity;
};

struct JsonObject
{
    struct JsonValue _header;
    struct JsonString *keys;
    struct JsonValue **values;
    unsigned int count;
    unsigned int capacity;
};

RAKU_LOCAL
void raku_json_bool_init(JsonBool *boolean);

RAKU_LOCAL
void raku_json_number_init(JsonNumber *number);

RAKU_LOCAL
void raku_json_string_init(JsonString *string);

RAKU_LOCAL
void raku_json_array_init(JsonArray *array);

RAKU_LOCAL
void raku_json_object_init(JsonObject *object);

RAKU_LOCAL
void raku_json_string_free(JsonString *string);

RAKU_LOCAL
void raku_json_array_free(JsonArray *array);

RAKU_LOCAL
void raku_json_object_free(JsonObject *object);

#endif