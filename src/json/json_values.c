#include "json_values.h"

#include <RAKU/core/memory.h>
#include <RAKU/debug.h>

#include <limits.h>
#include <string.h>
#include <stdio.h>

#define ARRAY_BASE_CAPACITY 8
#define OBJECT_BASE_CAPACITY 16

#define OBJECT_THRESHOLD 0.6

#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

static enum raku_status write_string(struct json_string *string, struct raku_string *out)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "write_string: invalid string.");

    enum raku_status status = raku_string_write(out, '"');
    if (status != RAKU_OK)
        goto ws_end;
    
    if (string->value.chars)
    {
        const char *c = string->value.chars;
        while (*c != '\0')
        {
            switch (*c)
            {
            case '"':
                status = raku_string_writesc(out, "\\\"");
                break;
            case '\\':
                status = raku_string_writesc(out, "\\\\");
                break;
            case '/':
                status = raku_string_writesc(out, "\\/");
                break;
            case '\b':
                status = raku_string_writesc(out, "\\b");
                break;
            case '\f':
                status = raku_string_writesc(out, "\\f");
                break;
            case '\n':
                status = raku_string_writesc(out, "\\n");
                break;
            case '\r':
                status = raku_string_writesc(out, "\\r");
                break;
            case '\t':
                status = raku_string_writesc(out, "\\t");
                break;
            default:
                status = raku_string_write(out, *c);
                break;
            }

            if (status != RAKU_OK)
                goto ws_end;
            
            ++c;
        }
    }

    status = raku_string_write(out, '"');

ws_end:
    return status;
}

static enum raku_status raku_json_value_to_string_compact(struct json_value *value, struct raku_string *out)
{
    switch (raku_json_value_get_type(value))
    {
        case RAKU_JSON_BOOL:
            if (((struct json_bool*)value)->value)
                return raku_string_writesc(out, "true");
            else
                return raku_string_writesc(out, "false");
        case RAKU_JSON_NUMBER:
        {
            char n[32];
            snprintf(n, 32, "%.24g", ((struct json_number*)value)->value);
            return raku_string_writesc(out, n);
        }
        default:
            ASSERT(false, "raku_json_value_to_string_compact: invalid json value.");
        case RAKU_JSON_NULL:
            return raku_string_writesc(out, "null");
        case RAKU_JSON_STRING:
            return write_string((struct json_string*)value, out);
        case RAKU_JSON_ARRAY:
        {
            enum raku_status status = raku_string_write(out, '[');
            if (status != RAKU_OK)
                goto rjvtsc_rjvgtv_caseRJA_end;
            
            struct json_array *array = (struct json_array*)value;
            if (array->count > 0)
            {
                status = raku_json_value_to_string_compact(array->values[0], out);
                if (status != RAKU_OK)
                    goto rjvtsc_rjvgtv_caseRJA_end;

                for (unsigned int i = 1; i < array->count; ++i)
                {
                    status = raku_string_write(out, ',');
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJA_end;
                    
                    status = raku_json_value_to_string_compact(array->values[i], out);
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJA_end;
                }
            }

            status = raku_string_write(out, ']');

        rjvtsc_rjvgtv_caseRJA_end:
            return status;
        }
        case RAKU_JSON_OBJECT:
        {
            enum raku_status status = raku_string_write(out, '{');
            if (status != RAKU_OK)
                goto rjvtsc_rjvgtv_caseRJO_end;
            
            struct json_object *object = (struct json_object*)value;
            if (object->count > 0)
            {
                unsigned int i = 0;
                unsigned int count = 0;
                for (; i < object->capacity && count != 1; ++i)
                {
                    if (object->keys[i].value.chars == NULL)
                        continue;

                    status = write_string(object->keys+i, out);
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;
                    
                    status = raku_string_write(out, ':');
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;
                    
                    status = raku_json_value_to_string_compact(object->values[i], out);
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;
                    
                    ++count;
                }

                for (; i < object->capacity && count < object->count; ++i)
                {
                    if (object->keys[i].value.chars == NULL)
                        continue;

                    status = raku_string_write(out, ',');
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;

                    status = write_string(object->keys+i, out);
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;
                    
                    status = raku_string_write(out, ':');
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;
                    
                    status = raku_json_value_to_string_compact(object->values[i], out);
                    if (status != RAKU_OK)
                        goto rjvtsc_rjvgtv_caseRJO_end;
                    
                    ++count;
                }
            }

            status = raku_string_write(out, '}');
            
        rjvtsc_rjvgtv_caseRJO_end:
            return status;
        }
    }
}

static enum raku_status write_indent(const char *indent, unsigned int level, struct raku_string *out)
{
    enum raku_status status = RAKU_OK;
    for (unsigned int i = 0; i < level; ++i)
    {
        status = raku_string_writesc(out, indent);
        if (status != RAKU_OK)
            break;
    }
    return status;
}

static enum raku_status raku_json_value_to_string_indent(
    struct json_value *value,
    const char *indent,
    unsigned int level,
    struct raku_string *out)
{
    switch (raku_json_value_get_type(value))
    {
        case RAKU_JSON_BOOL:
            if (((struct json_bool*)value)->value)
                return raku_string_writesc(out, "true");
            else
                return raku_string_writesc(out, "false");
        case RAKU_JSON_NUMBER:
        {
            char n[32];
            snprintf(n, 32, "%.24g", ((struct json_number*)value)->value);
            return raku_string_writesc(out, n);
        }
        default:
            ASSERT(false, "raku_json_value_to_string_compact: invalid json value.");
        case RAKU_JSON_NULL:
            return raku_string_writesc(out, "null");
        case RAKU_JSON_STRING:
            return write_string((struct json_string*)value, out);
        case RAKU_JSON_ARRAY:
        {
            enum raku_status status = raku_string_write(out, '[');
            if (status != RAKU_OK)
                goto rjvtsi_rjvgtv_caseRJA_end;
            
            struct json_array *array = (struct json_array*)value;
            if (array->count > 0)
            {
                status = raku_string_write(out, '\n');
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJA_end;

                status = write_indent(indent, level+1, out);
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJA_end;

                status = raku_json_value_to_string_indent(array->values[0], indent, level+1, out);
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJA_end;

                for (unsigned int i = 1; i < array->count; ++i)
                {
                    status = raku_string_writesc(out, ",\n");
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJA_end;

                    status = write_indent(indent, level+1, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJA_end;

                    status = raku_json_value_to_string_indent(array->values[i], indent, level+1, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJA_end;
                }

                status = raku_string_write(out, '\n');
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJA_end;

                status = write_indent(indent, level, out);
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJA_end;
            }

            status = raku_string_write(out, ']');

        rjvtsi_rjvgtv_caseRJA_end:
            return status;
        }
        case RAKU_JSON_OBJECT:
        {
            enum raku_status status = raku_string_write(out, '{');
            if (status != RAKU_OK)
                goto rjvtsi_rjvgtv_caseRJO_end;
            
            struct json_object *object = (struct json_object*)value;
            if (object->count > 0)
            {
                status = raku_string_write(out, '\n');
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJO_end;
                
                status = write_indent(indent, level+1, out);
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJO_end;

                unsigned int i = 0;
                unsigned int count = 0;
                for (; i < object->capacity && count != 1; ++i)
                {
                    if (object->keys[i].value.chars == NULL)
                        continue;

                    status = write_string(object->keys+i, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    status = raku_string_writesc(out, ": ");
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    status = raku_json_value_to_string_indent(object->values[i], indent, level+1, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    ++count;
                }

                for (; i < object->capacity && count < object->count; ++i)
                {
                    if (object->keys[i].value.chars == NULL)
                        continue;

                    status = raku_string_writesc(out, ",\n");
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    status = write_indent(indent, level+1, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;

                    status = write_string(object->keys+i, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    status = raku_string_writesc(out, ": ");
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    status = raku_json_value_to_string_indent(object->values[i], indent, level+1, out);
                    if (status != RAKU_OK)
                        goto rjvtsi_rjvgtv_caseRJO_end;
                    
                    ++count;
                }

                status = raku_string_write(out, '\n');
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJO_end;
                
                status = write_indent(indent, level, out);
                if (status != RAKU_OK)
                    goto rjvtsi_rjvgtv_caseRJO_end;
            }

            status = raku_string_write(out, '}');
            
        rjvtsi_rjvgtv_caseRJO_end:
            return status;
        }
    }
}

RAKU_API
enum raku_status raku_json_value_to_string(struct json_value *value, enum json_format_option options, struct raku_string *out)
{
    struct raku_string string;
    raku_string_init(&string);

    enum raku_status status = RAKU_OK;
    if ((options & 0x3) == RAKU_JSON_FORMAT_COMPACT)
        status = raku_json_value_to_string_compact(value, &string);
    else if ((options & 0x3) == RAKU_JSON_FORMAT_INDENT2)
        status = raku_json_value_to_string_indent(value, "  ", 0, &string);
    else if ((options & 0x3) == RAKU_JSON_FORMAT_INDENT4)
        status = raku_json_value_to_string_indent(value, "    ", 0, &string);
    else if ((options & 0x3) == RAKU_JSON_FORMAT_TAB)
        status = raku_json_value_to_string_indent(value, "\t", 0, &string);
    
    if (status == RAKU_OK)
        raku_string_own(out, &string);
    else
        raku_string_free(&string);
    
    return status;
}

RAKU_API
enum json_value_type raku_json_value_get_type(struct json_value *value)
{
    return value ? value->type : RAKU_JSON_NULL;
}

RAKU_API
bool raku_json_value_of_type(struct json_value *value, enum json_value_type type)
{
    return value ? value->type == type : type == RAKU_JSON_NULL;
}

RAKU_LOCAL
void raku_json_bool_init(struct json_bool *boolean)
{
    boolean->_header.type = RAKU_JSON_BOOL;
    boolean->value = 0;
}

RAKU_LOCAL
void raku_json_number_init(struct json_number *number)
{
    number->_header.type = RAKU_JSON_NUMBER;
    number->value = 0;
}

RAKU_LOCAL
void raku_json_string_init(struct json_string *string)
{
    string->_header.type = RAKU_JSON_STRING;
    string->hash = FNV_OFFSET_BASIS;
    raku_string_init(&string->value);
}

RAKU_LOCAL
void raku_json_array_init(struct json_array *array)
{
    array->_header.type = RAKU_JSON_ARRAY;
    array->values = NULL;
    array->count = 0;
    array->capacity = 0;
}

RAKU_LOCAL
void raku_json_object_init(struct json_object *object)
{
    object->_header.type = RAKU_JSON_OBJECT;
    object->keys = NULL;
    object->values = NULL;
    object->count = 0;
    object->capacity = 0;
}

RAKU_API
enum raku_status raku_json_bool_create(struct json_bool **out)
{
    struct json_bool *boolean;
    enum raku_status status = raku_alloc(
        sizeof(struct json_bool),
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
enum raku_status raku_json_number_create(struct json_number **out)
{
    struct json_number *number;
    enum raku_status status = raku_alloc(
        sizeof(struct json_number),
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
enum raku_status raku_json_string_create(struct json_string **out)
{
    struct json_string *string;
    enum raku_status status = raku_alloc(
        sizeof(struct json_string),
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
enum raku_status raku_json_array_create(struct json_array **out)
{
    struct json_array *array;
    enum raku_status status = raku_alloc(
        sizeof(struct json_array),
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
enum raku_status raku_json_object_create(struct json_object **out)
{
    struct json_object *object;
    enum raku_status status = raku_alloc(
        sizeof(struct json_object),
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
void raku_json_string_free(struct json_string *string)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "raku_json_string_free: invalid string.");

    raku_string_free(&string->value);
}

RAKU_LOCAL
void raku_json_array_free(struct json_array *array)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_free: invalid array.");

    for (unsigned int i = 0; i < array->count; ++i)
    {
        raku_json_value_free(array->values[i]);
    }
    raku_free(array->values);
}

RAKU_LOCAL
void raku_json_object_free(struct json_object *object)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_free: invalid object.");

    for (unsigned int i = 0, count = 0;
         i < object->capacity && count != object->count;
         ++i)
    {
        if (object->keys[i].value.chars == NULL)
            continue;
        raku_json_string_free(object->keys+i);
        raku_json_value_free(object->values[i]);
        ++count;
    }
    raku_free(object->keys);
    raku_free(object->values);
}

RAKU_API
void raku_json_value_free(struct json_value *value)
{
    switch (raku_json_value_get_type(value))
    {
        case RAKU_JSON_STRING:
            raku_json_string_free((struct json_string*)value);
            break;
        case RAKU_JSON_ARRAY:
            raku_json_array_free((struct json_array*)value);
            break;
        case RAKU_JSON_OBJECT:
            raku_json_object_free((struct json_object*)value);
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
void raku_json_bool_set(struct json_bool *boolean, bool value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)boolean, RAKU_JSON_BOOL),
           "raku_json_bool_set: invalid boolean.");
    boolean->value = value;
}

RAKU_API
bool raku_json_bool_get(struct json_bool *boolean)
{
    ASSERT(raku_json_value_of_type((struct json_value*)boolean, RAKU_JSON_BOOL),
           "raku_json_bool_get: invalid boolean.");
    return boolean->value;
}

RAKU_API
void raku_json_number_set(struct json_number *number, double value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)number, RAKU_JSON_NUMBER),
           "raku_json_number_set: invalid number.");
    number->value = value;
}

RAKU_API
double raku_json_number_get(struct json_number *number)
{
    ASSERT(raku_json_value_of_type((struct json_value*)number, RAKU_JSON_NUMBER),
           "raku_json_number_get: invalid number.");
    return number->value;
}

static string_hash hash_string(const char *src)
{
    string_hash hash = FNV_OFFSET_BASIS;
    if (src)
    {
        while (*src != '\0')
        {
            hash ^= *(src++);
            hash *= FNV_PRIME;
        }
    }
    return hash;
}

RAKU_API
void raku_json_string_set(struct json_string *string, struct raku_string *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "raku_json_string_set: invalid string.");
    ASSERT(value != NULL,
           "raku_json_string_set: value must not be NULL!");

    raku_string_own(&string->value, value);
    string->hash = hash_string(string->value.chars);
}

RAKU_API
enum raku_status raku_json_string_setc(struct json_string *string, const char *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "raku_json_string_setc: invalid string.");
    ASSERT(value != NULL,
           "raku_json_string_setc: value must not be NULL!");

    enum raku_status status = raku_string_copyc(&string->value, value);
    if (status == RAKU_OK)
        string->hash = hash_string(string->value.chars);
    
    return status;
}

RAKU_API
const struct raku_string raku_json_string_get(struct json_string *string)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "raku_json_string_get: invalid string.");
    
    return string->value;
}

RAKU_API
bool raku_json_string_equal(const struct json_string *string, const struct json_string *other)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "raku_json_string_equal: invalid string.");
    ASSERT(raku_json_value_of_type((struct json_value*)other, RAKU_JSON_STRING),
           "raku_json_string_equal: invalid other.");

    return
        (string->hash == other->hash) &&
        raku_string_equal(&string->value, &other->value);
}

RAKU_API
bool raku_json_string_equalc(const struct json_string *string, const char *other)
{
    ASSERT(raku_json_value_of_type((struct json_value*)string, RAKU_JSON_STRING),
           "raku_json_string_equalc: invalid string.");
    ASSERT(other != NULL,
           "raku_json_string_equalc: invalid other!");

    return raku_string_equalc(&string->value, other);
}

static enum raku_status grow_array(struct json_array *array, unsigned int size)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
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

    enum raku_status status = raku_realloc(
        array->values,
        new_capacity * sizeof(struct json_value*),
        (void**)&array->values
    );

    if (status == RAKU_OK)
        array->capacity = new_capacity;
    
    return status;
}

RAKU_API
enum raku_status raku_json_array_push(struct json_array *array, struct json_value *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push: invalid array.");

    enum raku_status status = RAKU_OK;
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
enum raku_status raku_json_array_push_bool(struct json_array *array, bool value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_bool: invalid array.");

    struct json_bool *boolean;
    enum raku_status status = raku_json_bool_create(&boolean);
    if (status == RAKU_OK)
    {
        raku_json_bool_set(boolean, value);
        status = raku_json_array_push(array, (struct json_value*)boolean);
        if (status != RAKU_OK)
            raku_json_value_free((struct json_value*)boolean);
    }
    return status;
}

RAKU_API
enum raku_status raku_json_array_push_number(struct json_array *array, double value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_number: invalid array.");

    struct json_number *number;
    enum raku_status status = raku_json_number_create(&number);
    if (status == RAKU_OK)
    {
        raku_json_number_set(number, value);
        status = raku_json_array_push(array, (struct json_value*)number);
        if (status != RAKU_OK)
            raku_json_value_free((struct json_value*)number);
    }
    return status;
}

RAKU_API
enum raku_status raku_json_array_push_string(struct json_array *array, struct raku_string *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_string: invalid array.");
    ASSERT(value != NULL,
           "raku_json_array_push_string: invalid value.");

    struct json_string *string;
    enum raku_status status = raku_json_string_create(&string);
    if (status == RAKU_OK)
    {
        raku_json_string_set(string, value);
        status = raku_json_array_push(array, (struct json_value*)string);
        if (status == RAKU_OK)
            goto rjaps_end;
        raku_json_value_free((struct json_value*)string);
    }

rjaps_end:
    return status;
}

RAKU_API
enum raku_status raku_json_array_push_stringc(struct json_array *array, const char *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_push_string: invalid array.");
    ASSERT(value != NULL,
           "raku_json_array_push_string: invalid value.");

    struct json_string *string;
    enum raku_status status = raku_json_string_create(&string);
    if (status == RAKU_OK)
    {
        status = raku_json_string_setc(string, value);
        if (status == RAKU_OK)
        {
            status = raku_json_array_push(array, (struct json_value*)string);
            if (status == RAKU_OK)
                goto rjapsc_end;
        }
        raku_json_value_free((struct json_value*)string);
    }

rjapsc_end:
    return status;
}

RAKU_API
void raku_json_array_remove(struct json_array *array, struct json_value *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
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
void raku_json_array_remove_bool(struct json_array *array, bool value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_bool: invalid array.");

    unsigned int index = 0;
    while (index < array->count)
    {
        struct json_value *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_BOOL) &&
            raku_json_bool_get((struct json_bool*)v) == value)
        {
            break;
        }
        ++index;
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
void raku_json_array_remove_number(struct json_array *array, double value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_number: invalid array.");

    unsigned int index = 0;
    while (index  < array->count)
    {
        struct json_value *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_NUMBER) &&
            raku_json_number_get((struct json_number*)v) == value)
        {
            break;
        }
        ++index;
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
void raku_json_array_remove_string(struct json_array *array, const struct raku_string *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_string: invalid array.");
    ASSERT(value != NULL,
           "raku_json_array_remove_string: invalid value.");

    unsigned int index = 0;
    while (index < array->count)
    {
        struct json_value *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_STRING) &&
            raku_string_equal(&((struct json_string*)v)->value, value))
        {
            break;
        }
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
void raku_json_array_remove_stringc(struct json_array *array, const char *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_remove_stringc: invalid array.");
    ASSERT(value != NULL,
           "raku_json_array_remove_stringc: invalid value.");

    unsigned int index = 0;
    while (index < array->count)
    {
        struct json_value *v = array->values[index];
        if (raku_json_value_of_type(v, RAKU_JSON_STRING) &&
            raku_json_string_equalc((struct json_string*)v, value))
        {
            break;
        }
    }
    raku_json_array_remove_at(array, index);
}

RAKU_API
enum raku_status raku_json_array_remove_at(struct json_array *array, unsigned int index)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
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
enum raku_status raku_json_array_get(struct json_array *array, unsigned int index, struct json_value **out)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_get: invalid array.");

    if (index >= array->count)
        return RAKU_OUT_OF_RANGE;
    
    *out = array->values[index];
    return RAKU_OK;
}

RAKU_API
struct json_value* const* raku_json_array_values(struct json_array *array)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_values: invalid array.");
    return array->values;
}

RAKU_API
unsigned int raku_json_array_size(struct json_array *array)
{
    ASSERT(raku_json_value_of_type((struct json_value*)array, RAKU_JSON_ARRAY),
           "raku_json_array_size: invalid array.");
    return array->count;
}

static enum raku_status grow_object(struct json_object *object)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
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
    
    struct json_string *keys = NULL;
    struct json_value **values = NULL;

    enum raku_status status = raku_alloc(
        new_capacity * sizeof(struct json_string),
        (void**)&keys
    );

    if (status == RAKU_OK)
    {
        status = raku_alloc(
            new_capacity * sizeof(struct json_value*),
            (void**)&values
        );

        if (status == RAKU_OK)
        {
            raku_zero_memory(keys, new_capacity * sizeof(struct json_string));
            raku_zero_memory(values, new_capacity * sizeof(struct json_value*));

            unsigned int count = 0;
            for (unsigned int i = 0;
                 i < object->capacity && count != object->count;
                 ++i)
            {
                struct json_string key = object->keys[i];
                if (key.value.chars == NULL)
                    continue;
                
                unsigned int index = key.hash % new_capacity;
                while (true)
                {
                    if (keys[index].value.chars == NULL)
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
enum raku_status raku_json_object_set(struct json_object *object, const char *key, struct json_value *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set: invalid object.");
    ASSERT(strnlen(key, UINT_MAX) != 0, "raku_json_object_set: invalid key.");

    enum raku_status status = RAKU_OK;
    if (object->count+1 > object->capacity * OBJECT_THRESHOLD)
    {
        status = grow_object(object);
        if (status != RAKU_OK)
            goto rjos_error;
    }

    struct json_string jskey;
    raku_json_string_init(&jskey);
    status = raku_json_string_setc(&jskey, key);
    if (status != RAKU_OK)
        goto rjos_error;
    
    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].value.chars == NULL)
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
enum raku_status raku_json_object_set_bool(struct json_object *object, const char *key, bool value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_bool: invalid object.");

    struct json_bool *boolean;
    enum raku_status status = raku_json_bool_create(&boolean);
    if (status == RAKU_OK)
    {
        raku_json_bool_set(boolean, value);
        status = raku_json_object_set(object, key, (struct json_value*)boolean);
        if (status != RAKU_OK)
            raku_json_value_free((struct json_value*)boolean);
    }
    return status;
}

RAKU_API
enum raku_status raku_json_object_set_number(struct json_object *object, const char *key, double value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_number: invalid object.");

    struct json_number *number;
    enum raku_status status = raku_json_number_create(&number);
    if (status == RAKU_OK)
    {
        raku_json_number_set(number, value);
        status = raku_json_object_set(object, key, (struct json_value*)number);
        if (status != RAKU_OK)
            raku_json_value_free((struct json_value*)number);
    }
    return status;
}

RAKU_API
enum raku_status raku_json_object_set_string(struct json_object *object, const char *key, struct raku_string *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_string: invalid object.");
    ASSERT(value != NULL,
           "raku_json_object_set_string: invalid value.");

    struct json_string *string;
    enum raku_status status = raku_json_string_create(&string);
    if (status == RAKU_OK)
    {
        raku_json_string_set(string, value);
        status = raku_json_object_set(object, key, (struct json_value*)string);
        if (status != RAKU_OK)
            raku_json_value_free((struct json_value*)string);
    }

    return status;
}

RAKU_API
enum raku_status raku_json_object_set_stringc(struct json_object *object, const char *key, const char *value)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_set_string: invalid object.");
    ASSERT(value != NULL,
           "raku_json_object_set_string: invalid value.");

    struct json_string *string;
    enum raku_status status = raku_json_string_create(&string);
    if (status == RAKU_OK)
    {
        status = raku_json_string_setc(string, value);
        if (status == RAKU_OK)
        {
            status = raku_json_object_set(object, key, (struct json_value*)string);
            if (status == RAKU_OK)
                goto rjossc_end;
        }
        raku_json_value_free((struct json_value*)string);
    }

rjossc_end:
    return status;
}

RAKU_API
void raku_json_object_remove(struct json_object *object, const char *key)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_remove: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_remove: invalid key.");

    const struct json_string jskey = {
        ._header.type = RAKU_JSON_STRING,
        .hash = hash_string(key),
        .value = {
            .chars = (char*)key,
            .count = size,
            .capacity = size
        }
    };

    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].value.chars == NULL)
            return;
        else if (raku_json_string_equal(object->keys+index, &jskey))
            break;
        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }

    raku_json_string_free(object->keys+index);
    raku_json_value_free(object->values[index]);
    --object->count;

    for (unsigned int i = index+1; object->keys[i].value.chars != NULL; ++i)
    {
        i = (i < object->capacity) ? i : i % object->capacity;

        struct json_string key = object->keys[i];
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
bool raku_json_object_has(struct json_object *object, const char *key)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_has: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_has: invalid key.");

    const struct json_string jskey = {
        ._header.type = RAKU_JSON_STRING,
        .hash = hash_string(key),
        .value = {
            .chars = (char*)key,
            .count = size,
            .capacity = size
        }
    };

    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].value.chars == NULL)
            return false;
        else if (raku_json_string_equal(object->keys+index, &jskey))
            return true;
        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }
}

RAKU_API
enum raku_status raku_json_object_get(struct json_object *object, const char *key, struct json_value **out)
{
    ASSERT(raku_json_value_of_type((struct json_value*)object, RAKU_JSON_OBJECT),
           "raku_json_object_get: invalid object.");

    unsigned int size = (unsigned int)strnlen(key, UINT_MAX);
    ASSERT(size != 0, "raku_json_object_get: invalid key.");

    const struct json_string jskey = {
        ._header.type = RAKU_JSON_STRING,
        .hash = hash_string(key),
        .value = {
            .chars = (char*)key,
            .count = size,
            .capacity = size
        }
    };

    unsigned int index = jskey.hash % object->capacity;
    while (true)
    {
        if (object->keys[index].value.chars == NULL)
            return RAKU_OUT_OF_RANGE;
        else if (raku_json_string_equal(object->keys+index, &jskey))
        {
            *out = object->values[index];
            return RAKU_OK;
        }
        
        ++index;
        index = (index < object->capacity) ? index : index % object->capacity;
    }
}