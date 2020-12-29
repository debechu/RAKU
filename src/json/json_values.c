#include "json_values.h"

#include <RAKU/core/memory.h>
#include <RAKU/debug.h>

#include <limits.h>
#include <string.h>

#define STRING_BASE_CAPACITY 8
#define ARRAY_BASE_CAPACITY 8
#define OBJECT_BASE_CAPACITY 16

#define OBJECT_THRESHOLD 0.6

#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

RAKU_API
JsonValueType raku_json_value_get_type(JsonValue *value)
{
    return value ? value->type : RAKU_JSON_NULL;
}

RAKU_API
bool raku_json_value_of_type(JsonValue *value, JsonValueType type)
{
    return value ? value->type == type : type == RAKU_JSON_NULL;
}

RAKU_LOCAL
void raku_json_bool_init(JsonBool *boolean)
{
    boolean->_header.type = RAKU_JSON_BOOL;
    boolean->value = 0;
}

RAKU_LOCAL
void raku_json_number_init(JsonNumber *number)
{
    number->_header.type = RAKU_JSON_NUMBER;
    number->value = 0;
}

RAKU_LOCAL
void raku_json_string_init(JsonString *string)
{
    string->_header.type = RAKU_JSON_STRING;
    string->chars = NULL;
    string->hash = FNV_OFFSET_BASIS;
    string->count = 0;
    string->capacity = 0;
}

RAKU_LOCAL
void raku_json_array_init(JsonArray *array)
{
    array->_header.type = RAKU_JSON_ARRAY;
    array->values = NULL;
    array->count = 0;
    array->capacity = 0;
}

RAKU_LOCAL
void raku_json_object_init(JsonObject *object)
{
    object->_header.type = RAKU_JSON_OBJECT;
    object->keys = NULL;
    object->values = NULL;
    object->count = 0;
    object->capacity = 0;
}

RAKU_API
RakuStatus raku_json_bool_create(JsonBool **out)
{
    JsonBool *boolean;
    RakuStatus status = raku_alloc(
        sizeof(struct JsonBool),
        (void**)&boolean
    );

    if (status == RAKU_OK)
    {
        raku_json_bool_init(boolean);
        *out = boolean;
    }
    
    return status;
}

RAKU_API
RakuStatus raku_json_number_create(JsonNumber **out)
{
    JsonNumber *number;
    RakuStatus status = raku_alloc(
        sizeof(struct JsonNumber),
        (void**)&number
    );

    if (status == RAKU_OK)
    {
        raku_json_number_init(number);
        *out = number;
    }

    return status;
}

RAKU_API
RakuStatus raku_json_string_create(JsonString **out)
{
    JsonString *string;
    RakuStatus status = raku_alloc(
        sizeof(struct JsonString),
        (void**)&string
    );

    if (status == RAKU_OK)
    {
        raku_json_string_init(string);
        *out = string;
    }

    return status;
}

RAKU_API
RakuStatus raku_json_array_create(JsonArray **out)
{
    JsonArray *array;
    RakuStatus status = raku_alloc(
        sizeof(struct JsonArray),
        (void**)&array
    );

    if (status == RAKU_OK)
    {
        raku_json_array_init(array);
        *out = array;
    }

    return status;
}

RAKU_API
RakuStatus raku_json_object_create(JsonObject **out)
{
    JsonObject *object;
    RakuStatus status = raku_alloc(
        sizeof(struct JsonObject),
        (void**)&object
    );

    if (status == RAKU_OK)
    {
        raku_json_object_init(object);
        *out = object;
    }

    return status;
}

RAKU_LOCAL
void raku_json_string_free(JsonString *string)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_free: invalid string.");

    raku_free(string->chars);
}

RAKU_LOCAL
void raku_json_array_free(JsonArray *array)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_free: invalid array.");

    for (unsigned int i = 0; i < array->count; ++i)
    {
        raku_json_value_free(array->values[i]);
    }
    raku_free(array->values);
}

RAKU_LOCAL
void raku_json_object_free(JsonObject *object)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_free: invalid object.");

    for (unsigned int i = 0, count = 0;
         i < object->capacity && count != object->count;
         ++i)
    {
        if (object->keys[i].chars == NULL)
            continue;
        raku_json_string_free(object->keys+i);
        raku_json_value_free(object->values[i]);
        ++count;
    }
    raku_free(object->keys);
    raku_free(object->values);
}

RAKU_API
void raku_json_value_free(JsonValue *value)
{
    switch (raku_json_value_get_type(value))
    {
        case RAKU_JSON_STRING:
            raku_json_string_free((JsonString*)value);
            break;
        case RAKU_JSON_ARRAY:
            raku_json_array_free((JsonArray*)value);
            break;
        case RAKU_JSON_OBJECT:
            raku_json_object_free((JsonObject*)value);
            break;
        case RAKU_JSON_NULL:
        case RAKU_JSON_BOOL:
        case RAKU_JSON_NUMBER:
            break;
        default:
            ASSERT(false, "raku_json_value_free: invalid json value.");
            break;
    }
    raku_free(value);
}

RAKU_API
void raku_json_bool_set(JsonBool *boolean, bool value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)boolean, RAKU_JSON_BOOL),
           "raku_json_bool_set: invalid boolean.");
    boolean->value = value;
}

RAKU_API
bool raku_json_bool_get(JsonBool *boolean)
{
    ASSERT(raku_json_value_of_type((JsonValue*)boolean, RAKU_JSON_BOOL),
           "raku_json_bool_get: invalid boolean.");
    return boolean->value;
}

RAKU_API
void raku_json_number_set(JsonNumber *number, double value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)number, RAKU_JSON_NUMBER),
           "raku_json_number_set: invalid number.");
    number->value = value;
}

RAKU_API
double raku_json_number_get(JsonNumber *number)
{
    ASSERT(raku_json_value_of_type((JsonValue*)number, RAKU_JSON_NUMBER),
           "raku_json_number_get: invalid number.");
    return number->value;
}

static StringHash hash_string(const char *src)
{
    StringHash hash = FNV_OFFSET_BASIS;
    while (*src != '\0')
    {
        hash ^= *(src++);
        hash *= FNV_PRIME;
    }
    return hash;
}

static RakuStatus grow_string(JsonString *string, unsigned int size)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "grow_string: invalid string.");
    
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
    
    RakuStatus status = raku_realloc(
        string->chars,
        (new_capacity+1) * sizeof(char),
        (void**)&string->chars
    );

    if (status == RAKU_OK)
        string->capacity = new_capacity;
    
    return status;
}

RAKU_API
RakuStatus raku_json_string_write(JsonString *string, char c)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_write: invalid string.");

    RakuStatus status = RAKU_OK;
    if (string->count+1 > string->capacity)
    {
        status = grow_string(string, 1U);
        if (status != RAKU_OK)
            goto rjsw_error;
    }

    string->chars[string->count++] = c;
    string->chars[string->count] = '\0';
    string->hash ^= c;
    string->hash *= FNV_PRIME;

rjsw_error:
    return status;
}

RAKU_API
RakuStatus raku_json_string_write_string(JsonString *string, JsonString *other)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_write_string: invalid string.");
    ASSERT(raku_json_value_of_type((JsonValue*)other, RAKU_JSON_STRING),
           "raku_json_string_write_string: invalid other.");
    
    
    RakuStatus status = RAKU_OK;
    if (string->count+other->count > string->capacity)
    {
        status = grow_string(string, other->count);
        if (status != RAKU_OK)
            goto rjsws_error;
    }

    for (unsigned int i = 0; i < other->count; ++i)
    {
        char c = other->chars[i];
        string->chars[string->count++] = c;
        string->hash ^= c;
        string->hash *= FNV_PRIME;
    }
    string->chars[string->count] = '\0';

rjsws_error:
    return status;
}

RAKU_API
RakuStatus raku_json_string_write_stringc(JsonString *string, unsigned int size, const char *other)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_write_stringc: invalid string.");
    ASSERT(other != NULL,
           "raku_json_string_write_stringc: invalid other.");

    RakuStatus status = RAKU_OK;
    if (string->count+size > string->capacity)
    {
        status = grow_string(string, size);
        if (status != RAKU_OK)
            goto rjswsc_error;
    }

    for (unsigned int i = 0; i < size; ++i)
    {
        char c = other[i];
        string->chars[string->count++] = c;
        string->hash ^= c;
        string->hash *= FNV_PRIME;
    }
    string->chars[string->count] = '\0';

rjswsc_error:
    return status;
}

RAKU_API
RakuStatus raku_json_string_get(JsonString *string, unsigned int index, char *out)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_get: invalid string.");
    
    if (index >= string->count)
        return RAKU_OUT_OF_RANGE;

    *out = string->chars[index];
    return RAKU_OK;
}

RAKU_API
bool raku_json_string_equal(JsonString *string, JsonString *other)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_equal: invalid string.");
    ASSERT(raku_json_value_of_type((JsonValue*)other, RAKU_JSON_STRING),
           "raku_json_string_equal: invalid other.");

    return
        (string->count == other->count)
        && (string->hash == other->hash)
        && (strncmp(string->chars, other->chars, string->count) == 0);
}

RAKU_API
bool raku_json_string_equalc(JsonString *string, unsigned int size, const char *other)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_equal: invalid string.");

    return
        (string->count == size)
        && (strncmp(string->chars, other, size) == 0);
}

RAKU_API
const char* raku_json_string_chars(JsonString *string)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_chars: invalid string.");
    return string->chars;
}

RAKU_API
unsigned int raku_json_string_size(JsonString *string)
{
    ASSERT(raku_json_value_of_type((JsonValue*)string, RAKU_JSON_STRING),
           "raku_json_string_size: invalid string.");
    return string->count;
}

static RakuStatus grow_array(JsonArray *array, unsigned int size)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "grow_array: invalid array.");

    if (size == 0)
        return RAKU_OK;
    else if (size > (UINT_MAX - array->count - 8))
        return RAKU_NO_MEMORY;
    
    unsigned int new_capacity;
    unsigned int min_capacity = array->count + size;
    if (min_capacity > (UINT_MAX / 2))
        new_capacity = min_capacity+8;
    else
    {
        new_capacity =
            (array->capacity < ARRAY_BASE_CAPACITY) ?
                ARRAY_BASE_CAPACITY :
                2 * array->capacity;
        if (new_capacity < min_capacity)
            new_capacity = min_capacity;
    }

    RakuStatus status = raku_realloc(
        array->values,
        new_capacity * sizeof(JsonValue*),
        (void**)&array->values
    );

    if (status == RAKU_OK)
        array->capacity = new_capacity;
    
    return status;
}

RAKU_API
RakuStatus raku_json_array_push(JsonArray *array, JsonValue *value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push: invalid array.");

    RakuStatus status = RAKU_OK;
    if (array->count+1 > array->capacity)
    {
        status = grow_array(array, 1U);
        if (status != RAKU_OK)
            goto rjap_error;
    }

    array->values[array->count++] = value;

rjap_error:
    return status;
}

RAKU_API
RakuStatus raku_json_array_push_bool(JsonArray *array, bool value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_bool: invalid array.");

    JsonBool *boolean;
    RakuStatus status = raku_json_bool_create(&boolean);
    if (status == RAKU_OK)
    {
        raku_json_bool_set(boolean, value);
        status = raku_json_array_push(array, (JsonValue*)boolean);
        if (status != RAKU_OK)
            raku_json_value_free((JsonValue*)boolean);
    }
    return status;
}

RAKU_API
RakuStatus raku_json_array_push_number(JsonArray *array, double value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_number: invalid array.");

    JsonNumber *number;
    RakuStatus status = raku_json_number_create(&number);
    if (status == RAKU_OK)
    {
        raku_json_number_set(number, value);
        status = raku_json_array_push(array, (JsonValue*)number);
        if (status != RAKU_OK)
            raku_json_value_free((JsonValue*)number);
    }
    return status;
}

RAKU_API
RakuStatus raku_json_array_push_string(JsonArray *array, unsigned int size, const char *value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_string: invalid array.");

    JsonString *string;
    RakuStatus status = raku_json_string_create(&string);
    if (status == RAKU_OK)
    {
        status = raku_json_string_write_stringc(string, size, value);
        if (status == RAKU_OK)
        {
            status = raku_json_array_push(array, (JsonValue*)string);
            if (status == RAKU_OK)
                goto rjaps_end;
        }
        raku_json_value_free((JsonValue*)string);
    }
rjaps_end:
    return status;
}

RAKU_API
void raku_json_array_remove(JsonArray *array, JsonValue *value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove: invalid array.");

    unsigned int index = 0;
    while (index < array->count)
    {
        if (array->values[index] == value)
            break;
        ++index;
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
void raku_json_array_remove_bool(JsonArray *array, bool value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_bool: invalid array.");

    unsigned int index = 0;
    while (index < array->count)
    {
        JsonValue *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_BOOL) &&
            raku_json_bool_get((JsonBool*)v) == value)
        {
            break;
        }
        ++index;
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
void raku_json_array_remove_number(JsonArray *array, double value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_number: invalid array.");

    unsigned int index = 0;
    while (index  < array->count)
    {
        JsonValue *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_NUMBER) &&
            raku_json_number_get((JsonNumber*)v) == value)
        {
            break;
        }
        ++index;
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
void raku_json_array_remove_string(JsonArray *array, unsigned int size, const char *value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_string: invalid array.");

    unsigned int index = 0;
    while (index < array->count)
    {
        JsonValue *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_STRING) &&
            raku_json_string_equalc((JsonString*)v, size, value))
        {
            break;
        }
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
RakuStatus raku_json_array_remove_at(JsonArray *array, unsigned int index)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_at: invalid array.");

    if (index >= array->count)
        return RAKU_OUT_OF_RANGE;
    
    raku_json_value_free(array->values[index]);
    --array->count;
    for (unsigned int i = index; i < array->count; ++i)
    {
        array->values[i] = array->values[i+1];
    }
    return RAKU_OK;
}

RAKU_API
RakuStatus raku_json_array_get(JsonArray *array, unsigned int index, JsonValue **out)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_get: invalid array.");

    if (index >= array->count)
        return RAKU_OUT_OF_RANGE;
    
    *out = array->values[index];
    return RAKU_OK;
}

RAKU_API
JsonValue* const* raku_json_array_values(JsonArray *array)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_values: invalid array.");
    return array->values;
}

RAKU_API
unsigned int raku_json_array_size(JsonArray *array)
{
    ASSERT(raku_json_value_of_type((JsonValue*)array, RAKU_JSON_ARRAY),
           "raku_json_array_size: invalid array.");
    return array->count;
}

static RakuStatus grow_object(JsonObject *object)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "grow_object: invalid object.");

    unsigned int new_capacity;
    if (object->capacity == UINT_MAX)
        return RAKU_NO_MEMORY;
    else if (object->capacity > (UINT_MAX / 2))
        new_capacity = UINT_MAX;
    else
    {
        new_capacity =
            (object->capacity < OBJECT_BASE_CAPACITY) ?
                OBJECT_BASE_CAPACITY :
                2 * object->capacity;
    }
    
    JsonString *keys = NULL;
    JsonValue **values = NULL;

    RakuStatus status = raku_alloc(
        new_capacity * sizeof(JsonString),
        (void**)&keys
    );

    if (status == RAKU_OK)
    {
        status = raku_alloc(
            new_capacity * sizeof(JsonValue*),
            (void**)&values
        );

        if (status == RAKU_OK)
        {
            raku_zero_memory(keys, new_capacity * sizeof(JsonString));
            raku_zero_memory(values, new_capacity * sizeof(JsonValue*));

            unsigned int count = 0;
            for (unsigned int i = 0;
                 i < object->capacity && count != object->count;
                 ++i)
            {
                JsonString key = object->keys[i];
                if (key.chars == NULL)
                    continue;
                
                unsigned int index = key.hash % new_capacity;
                while (true)
                {
                    if (keys[index].chars == NULL)
                    {
                        keys[index] = key;
                        values[index] = object->values[i];
                        break;
                    }

                    ++index;
                    index = (index < new_capacity) ? index : index % new_capacity;
                }
                ++count;
            }

            raku_free(object->keys);
            raku_free(object->values);

            object->keys = keys;
            object->values = values;
            object->count = count;
            object->capacity = new_capacity;
        }

        else
        {
            raku_free(keys);
        }
    }

    return status;
}

RAKU_API
RakuStatus raku_json_object_set(JsonObject *object, const char *key, JsonValue *value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_set: invalid key.");

    RakuStatus status = RAKU_OK;
    if (object->count+1 > object->capacity * OBJECT_THRESHOLD)
    {
        status = grow_object(object);
        if (status != RAKU_OK)
            goto rjos_error;
    }

    JsonString jskey;
    raku_json_string_init(&jskey);
    status = raku_json_string_write_stringc(&jskey, size, key);
    if (status != RAKU_OK)
        goto rjos_error;
    
    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].chars == NULL)
        {
            object->keys[index] = jskey;
            object->values[index] = value;
            ++object->count;
            break;
        }

        else if (raku_json_string_equal(object->keys+index, &jskey))
        {
            raku_json_value_free(object->values[index]);
            object->values[index] = value;
            raku_json_string_free(&jskey);
            break;
        }

        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }

rjos_error:
    return status;
}

RAKU_API
RakuStatus raku_json_object_set_bool(JsonObject *object, const char *key, bool value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_bool: invalid object.");

    JsonBool *boolean;
    RakuStatus status = raku_json_bool_create(&boolean);
    if (status == RAKU_OK)
    {
        raku_json_bool_set(boolean, value);
        status = raku_json_object_set(object, key, (JsonValue*)boolean);
        if (status != RAKU_OK)
            raku_json_value_free((JsonValue*)boolean);
    }
    return status;
}

RAKU_API
RakuStatus raku_json_object_set_number(JsonObject *object, const char *key, double value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_number: invalid object.");

    JsonNumber *number;
    RakuStatus status = raku_json_number_create(&number);
    if (status == RAKU_OK)
    {
        raku_json_number_set(number, value);
        status = raku_json_object_set(object, key, (JsonValue*)number);
        if (status != RAKU_OK)
            raku_json_value_free((JsonValue*)number);
    }
    return status;
}

RAKU_API
RakuStatus raku_json_object_set_string(JsonObject *object, const char *key, unsigned int size, const char *value)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_string: invalid object.");

    JsonString *string;
    RakuStatus status = raku_json_string_create(&string);
    if (status == RAKU_OK)
    {
        status = raku_json_string_write_stringc(string, size, value);
        if (status == RAKU_OK)
        {
            status = raku_json_object_set(object, key, (JsonValue*)string);
            if (status == RAKU_OK)
                goto rjoss_end;
        }
        raku_json_value_free((JsonValue*)string);
    }

rjoss_end:
    return status;
}

RAKU_API
void raku_json_object_remove(JsonObject *object, const char *key)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_remove: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_remove: invalid key.");

    JsonString jskey = {
        ._header.type = RAKU_JSON_STRING,
        .chars = (char*)key,
        .hash = hash_string(key),
        .count = size,
        .capacity = size
    };

    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].chars == NULL)
            return;
        else if (raku_json_string_equal(object->keys+index, &jskey))
            break;
        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }

    raku_json_string_free(object->keys+index);
    raku_json_value_free(object->values[index]);
    --object->count;

    for (unsigned int i = index+1; object->keys[i].chars != NULL; ++i)
    {
        i = (i < object->capacity) ? i : i % object->capacity;

        JsonString key = object->keys[i];
        if ((key.hash % object->capacity) <= index)
        {
            object->keys[index] = key;
            object->values[index] = object->values[i];
            index = i;
        }
    }

    raku_json_string_init(object->keys+index);
    object->values[index] = NULL;
}

RAKU_API
bool raku_json_object_has(JsonObject *object, const char *key)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_has: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_has: invalid key.");

    JsonString jskey = {
        ._header.type = RAKU_JSON_STRING,
        .chars = (char*)key,
        .hash = hash_string(key),
        .count = size,
        .capacity = size
    };

    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].chars == NULL)
            return false;
        else if (raku_json_string_equal(object->keys+index, &jskey))
            return true;
        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }
}

RAKU_API
RakuStatus raku_json_object_get(JsonObject *object, const char *key, JsonValue **out)
{
    ASSERT(raku_json_value_of_type((JsonValue*)object, RAKU_JSON_OBJECT),
           "raku_json_object_get: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_get: invalid key.");

    JsonString jskey = {
        ._header.type = RAKU_JSON_STRING,
        .chars = (char*)key,
        .hash = hash_string(key),
        .count = size,
        .capacity = size
    };

    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].chars == NULL)
            return RAKU_JSON_NO_KEY;
        else if (raku_json_string_equal(object->keys+index, &jskey))
        {
            *out = object->values[index];
            return RAKU_OK;
        }
        
        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }
}