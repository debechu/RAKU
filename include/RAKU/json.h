#ifndef RAKU_JSON_H
#define RAKU_JSON_H

#include <RAKU/export.h>
#include <RAKU/core/defs.h>
#include <RAKU/core/status.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum JsonValueType
{
    RAKU_JSON_NULL,
    RAKU_JSON_BOOL,
    RAKU_JSON_NUMBER,
    RAKU_JSON_STRING,
    RAKU_JSON_ARRAY,
    RAKU_JSON_OBJECT
} JsonValueType;

typedef struct JsonValue JsonValue;
typedef struct JsonBool JsonBool;
typedef struct JsonNumber JsonNumber;
typedef struct JsonString JsonString;
typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;

RAKU_API
JsonValueType raku_json_value_get_type(JsonValue *value);

RAKU_API
bool raku_json_value_of_type(JsonValue *value, JsonValueType type);

RAKU_API
RakuStatus raku_json_bool_create(JsonBool **out);

RAKU_API
RakuStatus raku_json_number_create(JsonNumber **out);

RAKU_API
RakuStatus raku_json_string_create(JsonString **out);

RAKU_API
RakuStatus raku_json_array_create(JsonArray **out);

RAKU_API
RakuStatus raku_json_object_create(JsonObject **out);

RAKU_API
void raku_json_value_free(JsonValue *value);

RAKU_API
void raku_json_bool_set(JsonBool *boolean, bool value);

RAKU_API
bool raku_json_bool_get(JsonBool *boolean);

RAKU_API
void raku_json_number_set(JsonNumber *number, double value);

RAKU_API
double raku_json_number_get(JsonNumber *number);

RAKU_API
RakuStatus raku_json_string_write(JsonString *string, char c);

RAKU_API
RakuStatus raku_json_string_write_string(JsonString *string, JsonString *other);

RAKU_API
RakuStatus raku_json_string_write_stringc(JsonString *string, unsigned int size, const char *other);

RAKU_API
RakuStatus raku_json_string_get(JsonString *string, unsigned int index, char *out);

RAKU_API
bool raku_json_string_equal(JsonString *string, JsonString *other);

RAKU_API
bool raku_json_string_equalc(JsonString *string, unsigned int size, const char *other);

RAKU_API
const char* raku_json_string_chars(JsonString *string);

RAKU_API
unsigned int raku_json_string_size(JsonString *string);

RAKU_API
RakuStatus raku_json_array_push(JsonArray *array, JsonValue *value);

RAKU_API
RakuStatus raku_json_array_push_bool(JsonArray *array, bool value);

RAKU_API
RakuStatus raku_json_array_push_number(JsonArray *array, double value);

RAKU_API
RakuStatus raku_json_array_push_string(JsonArray *array, unsigned int size, const char *value);

RAKU_API
void raku_json_array_remove(JsonArray *array, JsonValue *value);

RAKU_API
void raku_json_array_remove_bool(JsonArray *array, bool value);

RAKU_API
void raku_json_array_remove_number(JsonArray *array, double value);

RAKU_API
void raku_json_array_remove_string(JsonArray *array, unsigned int size, const char *value);

RAKU_API
RakuStatus raku_json_array_remove_at(JsonArray *array, unsigned int index);

RAKU_API
RakuStatus raku_json_array_get(JsonArray *array, unsigned int index, JsonValue **out);

RAKU_API
JsonValue* const* raku_json_array_values(JsonArray *array);

RAKU_API
unsigned int raku_json_array_size(JsonArray *array);

RAKU_API
RakuStatus raku_json_object_set(JsonObject *object, const char *key, JsonValue *value);

RAKU_API
RakuStatus raku_json_object_set_bool(JsonObject *object, const char *key, bool value);

RAKU_API
RakuStatus raku_json_object_set_number(JsonObject *object, const char *key, double value);

RAKU_API
RakuStatus raku_json_object_set_string(JsonObject *object, const char *key, unsigned int size, const char *value);

RAKU_API
void raku_json_object_remove(JsonObject *object, const char *key);

RAKU_API
bool raku_json_object_has(JsonObject *object, const char *key);

RAKU_API
RakuStatus raku_json_object_get(JsonObject *object, const char *key, JsonValue **out);

#if defined(__cplusplus)
}
#endif

#endif