#ifndef RAKU_JSON_H
#define RAKU_JSON_H

#include <RAKU/export.h>
#include <RAKU/string.h>
#include <RAKU/core/defs.h>
#include <RAKU/core/status.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum json_value_type
{
    RAKU_JSON_NULL,
    RAKU_JSON_BOOL,
    RAKU_JSON_NUMBER,
    RAKU_JSON_STRING,
    RAKU_JSON_ARRAY,
    RAKU_JSON_OBJECT
};

enum json_format_option
{
    RAKU_JSON_FORMAT_COMPACT = 0,
    RAKU_JSON_FORMAT_INDENT2 = 1,
    RAKU_JSON_FORMAT_INDENT4 = 2,
    RAKU_JSON_FORMAT_TAB
};

struct json_value;
struct json_bool;
struct json_number;
struct json_string;
struct json_array;
struct json_object;

RAKU_API
enum raku_status raku_json_value_to_string(struct json_value *value, enum json_format_option options, struct raku_string *out);

RAKU_API
enum json_value_type raku_json_value_get_type(struct json_value *value);

RAKU_API
bool raku_json_value_of_type(struct json_value *value, enum json_value_type type);

RAKU_API
enum raku_status raku_json_bool_create(struct json_bool **out);

RAKU_API
enum raku_status raku_json_number_create(struct json_number **out);

RAKU_API
enum raku_status raku_json_string_create(struct json_string **out);

RAKU_API
enum raku_status raku_json_array_create(struct json_array **out);

RAKU_API
enum raku_status raku_json_object_create(struct json_object **out);

RAKU_API
void raku_json_value_free(struct json_value *value);

RAKU_API
void raku_json_bool_set(struct json_bool *boolean, bool value);

RAKU_API
bool raku_json_bool_get(struct json_bool *boolean);

RAKU_API
void raku_json_number_set(struct json_number *number, double value);

RAKU_API
double raku_json_number_get(struct json_number *number);

RAKU_API
void raku_json_string_set(struct json_string *string, struct raku_string *value);

RAKU_API
enum raku_status raku_json_string_setc(struct json_string *string, const char *value);

RAKU_API
const struct raku_string raku_json_string_get(struct json_string *string);

RAKU_API
bool raku_json_string_equal(const struct json_string *string, const struct json_string *other);

RAKU_API
bool raku_json_string_equalc(const struct json_string *string, const char *other);

RAKU_API
enum raku_status raku_json_array_push(struct json_array *array, struct json_value *value);

RAKU_API
enum raku_status raku_json_array_push_bool(struct json_array *array, bool value);

RAKU_API
enum raku_status raku_json_array_push_number(struct json_array *array, double value);

RAKU_API
enum raku_status raku_json_array_push_string(struct json_array *array, struct raku_string *value);

RAKU_API
enum raku_status raku_json_array_push_stringc(struct json_array *array, const char *value);

RAKU_API
void raku_json_array_remove(struct json_array *array, struct json_value *value);

RAKU_API
void raku_json_array_remove_bool(struct json_array *array, bool value);

RAKU_API
void raku_json_array_remove_number(struct json_array *array, double value);

RAKU_API
void raku_json_array_remove_string(struct json_array *array, const struct raku_string *value);

RAKU_API
void raku_json_array_remove_stringc(struct json_array *array, const char *value);

RAKU_API
enum raku_status raku_json_array_remove_at(struct json_array *array, unsigned int index);

RAKU_API
enum raku_status raku_json_array_get(struct json_array *array, unsigned int index, struct json_value **out);

RAKU_API
struct json_value* const* raku_json_array_values(struct json_array *array);

RAKU_API
unsigned int raku_json_array_size(struct json_array *array);

RAKU_API
enum raku_status raku_json_object_set(struct json_object *object, const char *key, struct json_value *value);

RAKU_API
enum raku_status raku_json_object_set_bool(struct json_object *object, const char *key, bool value);

RAKU_API
enum raku_status raku_json_object_set_number(struct json_object *object, const char *key, double value);

RAKU_API
enum raku_status raku_json_object_set_string(struct json_object *object, const char *key, struct raku_string *value);

RAKU_API
enum raku_status raku_json_object_set_stringc(struct json_object *object, const char *key, const char *value);

RAKU_API
void raku_json_object_remove(struct json_object *object, const char *key);

RAKU_API
bool raku_json_object_has(struct json_object *object, const char *key);

RAKU_API
enum raku_status raku_json_object_get(struct json_object *object, const char *key, struct json_value **out);

#if defined(__cplusplus)
}
#endif

#endif