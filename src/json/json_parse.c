#include <RAKU/json.h>
#include "json_values.h"
#include <RAKU/debug.h>

#include <ctype.h>
#include <string.h>
#include <math.h>

#define STACK_CAPACITY 64

#define ERROR(c, r) ((struct json_error) { .column = (c), .row = (r) })

struct json_parser
{
    struct lexer
    {
        const char *start;
        const char *current;
        unsigned int column;
        unsigned int row;
    } lexer;
};

void json_parser_init(struct json_parser *parser, const char *src)
{
    parser->lexer.start = src;
    parser->lexer.current = src;
    parser->lexer.column = 1;
    parser->lexer.row = 1;
}

static inline bool at_end(struct lexer *lexer)
{
    return *lexer->current == '\0';
}

static inline char advance(struct lexer *lexer)
{
    return ++lexer->column, *(lexer->current++);
}

static inline char peek(struct lexer *lexer)
{
    return *lexer->current;
}

static inline char peek_next(struct lexer *lexer)
{
    return lexer->current[1];
}

static inline bool is_hex(const char c)
{
    return
        (c >= '0' && c <= '9') ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F');
}

static inline bool is_char(const char c)
{
    return
        (c < 0) ||
        (c == 0x20) ||
        (c == 0x21) ||
        (c > 0x22);
}

static inline bool is_word(const char *start, unsigned int count, const char *comp)
{
    return (strncmp(start, comp, count) == 0);
}

static inline int get_digit_value(const char c)
{
    return c - '0';
}

static unsigned char get_hex_value(const char c)
{
    return
        (c < 'A') ?
            (c - '0') :
        (c < 'a') ?
            (c - 'A') + 10 :
            (c - 'a') + 10;
}

static void skip_whitespaces(struct lexer *lexer)
{
    while (true)
    {
        switch (*lexer->current)
        {
            case 0x0A:
                ++lexer->row;
                lexer->column = 0;
            case 0x09:
            case 0x0D:
            case 0x20:
                advance(lexer);
                continue;
        }

        break;
    }

    lexer->start = lexer->current;
}

static enum raku_status get_utf16(struct lexer *lexer, uint16_t *out)
{
    enum raku_status status = RAKU_OK;

    uint16_t res = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (!is_hex(peek(lexer)))
        {
            status = RAKU_JSON_INVALID_HEX;
            goto gu16_end;
        }

        res = (res * 16) + get_hex_value(advance(lexer));
    }

    *out = res;

gu16_end:
    return status;
}

static enum raku_status parse_value(struct json_parser *parser, struct json_value **out);

static enum raku_status parse_string(struct json_parser *parser, struct json_value **out)
{
    struct raku_string string;
    raku_string_init(&string);

    enum raku_status status = RAKU_OK;
    while (is_char(peek(&parser->lexer)))
    {
        char c = advance(&parser->lexer);
        if (c == '\\')
        {
            c = advance(&parser->lexer);
            switch (c)
            {
                case '"':
                case '/':
                case '\\':
                    status = raku_string_write(&string, c);
                    break;
                case 'b':
                    status = raku_string_write(&string, '\b');
                    break;
                case 'f':
                    status = raku_string_write(&string, '\f');
                    break;
                case 'n':
                    status = raku_string_write(&string, '\n');
                    break;
                case 'r':
                    status = raku_string_write(&string, '\r');
                    break;
                case 't':
                    status = raku_string_write(&string, '\t');
                    break;
                case 'x':
                {
                    uint32_t unicode;
                    status = get_utf16(&parser->lexer, (uint16_t*)&unicode+1);
                    if (status != RAKU_OK)
                        goto ps_end;
                    
                    if ((unicode & 0xFC00) == 0xD800 &&
                        peek(&parser->lexer) == '\\' &&
                        peek_next(&parser->lexer) == 'x')
                    {
                        advance(&parser->lexer); advance(&parser->lexer);

                        uint16_t trail;
                        status = get_utf16(&parser->lexer, &trail);
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        if ((trail & 0xFC00) != 0xDC00)
                        {
                            status = RAKU_JSON_INVALID_SURROGATE_PAIR;
                            goto ps_end;
                        }

                        unicode = ((unicode - 0xD800) << 10) | (trail - 0xDC00) + 0x10000;
                    }

                    if (unicode > 0x10FFFF)
                    {
                        status = RAKU_JSON_INVALID_CODE_POINT;
                        goto ps_end;
                    }

                    else if (unicode > 0xFFFF)
                    {
                        status = raku_string_write(&string, (char)((unicode >> 18) + 0xF0));
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        status = raku_string_write(&string, (char)(((unicode >> 12) & 0x3F) + 0x80));
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        status = raku_string_write(&string, (char)(((unicode >> 6) & 0x3F) + 0x80));
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        status = raku_string_write(&string, (char)((unicode & 0x3F) + 0x80));
                        if (status != RAKU_OK)
                            goto ps_end;
                    }

                    else if (unicode > 0x07FF)
                    {
                        status = raku_string_write(&string, (char)((unicode >> 12) + 0xE0));
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        status = raku_string_write(&string, (char)(((unicode >> 6) & 0x3F) + 0x80));
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        status = raku_string_write(&string, (char)((unicode & 0x3F) + 0x80));
                        if (status != RAKU_OK)
                            goto ps_end;
                    }

                    else if (unicode > 0x7F)
                    {
                        status = raku_string_write(&string, (char)((unicode >> 6) + 0xC0));
                        if (status != RAKU_OK)
                            goto ps_end;
                        
                        status = raku_string_write(&string, (char)((unicode & 0x3F) + 0x80));
                        if (status != RAKU_OK)
                            goto ps_end;
                    }

                    else
                    {
                        status = raku_string_write(&string, (char)unicode);
                        if (status != RAKU_OK)
                            goto ps_end;
                    }

                    break;
                }
                default:
                    status = RAKU_JSON_INVALID_ESCAPE_SEQUENCE;
                    break;
            }
        }

        else
            status = raku_string_write(&string, c);

        if (status != RAKU_OK)
            goto ps_end;
    }

    if (peek(&parser->lexer) == '"')
        advance(&parser->lexer);
    else
    {
        status = RAKU_JSON_UNTERMINATED_STRING;
    }

ps_end:
    if (status != RAKU_OK)
        raku_string_free(&string);
    else
    {
        struct json_string *value;
        status = raku_json_string_create(&value);
        if (status == RAKU_OK)
        {
            raku_json_string_set(value, &string);
            *out = (struct json_value*)value;
        }
    }

    return status;
}

static enum raku_status parse_number(struct json_parser *parser, struct json_value **out)
{
    enum raku_status status = RAKU_OK;
    parser->lexer.current = parser->lexer.start;
    --parser->lexer.column;

    double sign = 1;
    if (peek(&parser->lexer) == '-')
    {
        advance(&parser->lexer);
        if (!isdigit(peek(&parser->lexer)))
        {
            status = RAKU_JSON_INVALID_NUMBER;
            goto pn_end;
        }

        sign = -1;
    }
    
    double value = 0;
    while (isdigit(peek(&parser->lexer)))
    {
        value = (value * 10) + get_digit_value(advance(&parser->lexer));
    }
    
    if (peek(&parser->lexer) == '.')
    {
        int count = 0;
        double precision = 0;

        advance(&parser->lexer);
        if (!isdigit(peek(&parser->lexer)))
        {
            status = RAKU_JSON_MISSING_PRECISION;
            goto pn_end;
        }

        while (isdigit(peek(&parser->lexer)))
        {
            precision = (precision * 10) + get_digit_value(advance(&parser->lexer));
            --count;
        }

        for (; count < 0; ++count)
        {
            precision /= 10;
        }

        value += precision;
    }

    if (peek(&parser->lexer) == 'E' ||
        peek(&parser->lexer) == 'e')
    {
        double exp = 0;
        double sign = 1;

        advance(&parser->lexer);
        if (peek(&parser->lexer) == '-')
        {
            sign = -1;
            advance(&parser->lexer);
        }

        else if (peek(&parser->lexer) == '+')
            advance(&parser->lexer);
        
        if (!isdigit(peek(&parser->lexer)))
        {
            status = RAKU_JSON_MISSING_EXPONENT;
            goto pn_end;
        }

        while (isdigit(peek(&parser->lexer)))
        {
            exp = (exp * 10) + get_digit_value(advance(&parser->lexer));
        }

        double factor = 1;
        for (; exp > 0; --exp)
        {
            factor *= 10;
        }

        if (sign == -1)
            value /= factor;
        else
            value *= factor;
    }

    struct json_number *number;
    status = raku_json_number_create(&number);
    if (status == RAKU_OK)
    {
        raku_json_number_set(number, sign * value);
        *out = (struct json_value*)number;
    }

pn_end:
    return status;
}

static enum raku_status parse_array(struct json_parser *parser, struct json_value **out)
{
    struct json_array *array = NULL;
    enum raku_status status = raku_json_array_create(&array);
    if (status != RAKU_OK)
        goto pa_end1;

    skip_whitespaces(&parser->lexer);
    if (peek(&parser->lexer) != ']')
    {
        do
        {
            skip_whitespaces(&parser->lexer);

            struct json_value *value;
            status = parse_value(parser, &value);
            if (status != RAKU_OK)
                goto pa_end2;

            status = raku_json_array_push(array, value);
            if (status != RAKU_OK)
            {
                raku_json_value_free(value);
                goto pa_end2;
            }

            skip_whitespaces(&parser->lexer);
        } while (peek(&parser->lexer) == ',' && advance(&parser->lexer));
    }

    if (peek(&parser->lexer) != ']')
        status = RAKU_JSON_UNEXPECTED_SYMBOL;
    else
        advance(&parser->lexer);

pa_end2:
    if (status != RAKU_OK)
        raku_json_value_free((struct json_value*)array);
    else
        *out = (struct json_value*)array;

pa_end1:
    return status;
}

static enum raku_status parse_object(struct json_parser *parser, struct json_value **out)
{
    struct json_object *object = NULL;
    enum raku_status status = raku_json_object_create(&object);
    if (status != RAKU_OK)
        goto po_end1;
    
    skip_whitespaces(&parser->lexer);
    if (peek(&parser->lexer) != '}')
    {
        do
        {
            skip_whitespaces(&parser->lexer);
            if (peek(&parser->lexer) != '"')
            {
                status = RAKU_JSON_UNEXPECTED_SYMBOL;
                goto po_end2;
            }
            advance(&parser->lexer);

            struct json_string *key;
            status = parse_string(parser, (struct json_value**)&key);
            if (status != RAKU_OK)
                goto po_end2;
            
            skip_whitespaces(&parser->lexer);
            if (peek(&parser->lexer) != ':')
            {
                status = RAKU_JSON_UNEXPECTED_SYMBOL;
                raku_json_value_free((struct json_value*)key);
                goto po_end2;
            }
            advance(&parser->lexer);
            
            skip_whitespaces(&parser->lexer);

            struct json_value *value;
            status = parse_value(parser, &value);
            if (status != RAKU_OK)
            {
                raku_json_value_free((struct json_value*)key);
                goto po_end2;
            }

            status = raku_json_object_set(object, key->value.chars, value);
            raku_json_value_free((struct json_value*)key);
            if (status != RAKU_OK)
            {
                raku_json_value_free(value);
                goto po_end2;
            }

            skip_whitespaces(&parser->lexer);
        } while (peek(&parser->lexer) == ',' && advance(&parser->lexer));
    }

    if (peek(&parser->lexer) != '}')
        status = RAKU_JSON_UNEXPECTED_SYMBOL;
    else
        advance(&parser->lexer);
    
po_end2:
    if (status != RAKU_OK)
        raku_json_value_free((struct json_value*)object);
    else
        *out = (struct json_value*)object;
    
po_end1:
    return status;
}

static enum raku_status parse_value(struct json_parser *parser, struct json_value **out)
{
    skip_whitespaces(&parser->lexer);

    struct json_value *value = NULL;
    enum raku_status status = RAKU_OK;

    char c = advance(&parser->lexer);
    switch (c)
    {
        case 'n':
            if (!is_word(parser->lexer.current, 3, "ull"))
                status = RAKU_JSON_UNEXPECTED_SYMBOL;
            else
            {
                parser->lexer.current += 3;
                value = NULL;
            }
            break;
        case 'f':
            if (!is_word(parser->lexer.current, 4, "alse"))
                status = RAKU_JSON_UNEXPECTED_SYMBOL;
            else
            {
                parser->lexer.current += 4;
                status = raku_json_bool_create((struct json_bool**)&value);
                if (status != RAKU_OK)
                    goto pv_end;
                
                raku_json_bool_set((struct json_bool*)value, false);
            }
            break;
        case 't':
            if (!is_word(parser->lexer.current, 3, "rue"))
                status = RAKU_JSON_UNEXPECTED_SYMBOL;
            else
            {
                parser->lexer.current += 3;
                status = raku_json_bool_create((struct json_bool**)&value);
                if (status != RAKU_OK)
                    goto pv_end;
                
                raku_json_bool_set((struct json_bool*)value, true);
            }
            break;
        case '"':
            status = parse_string(parser, &value);
            break;
        case '0':
            if (isdigit(peek(&parser->lexer)))
            {
                status = RAKU_JSON_INVALID_NUMBER;
                goto pv_end;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            status = parse_number(parser, &value);
            break;
        case '[':
            status = parse_array(parser, &value);
            break;
        case '{':
            status = parse_object(parser, &value);
            break;
        default:
            status = RAKU_JSON_UNEXPECTED_SYMBOL;
            break;
    }

pv_end:
    if (status == RAKU_OK)
        *out = value;
    else
        raku_json_value_free(value);
    
    return status;
}

RAKU_API
enum raku_status raku_json_parse(const char *src, struct json_value **out)
{
    ASSERT(src != NULL,
           "raku_json_parse: src must not be NULL!");
    ASSERT(out != NULL,
           "raku_json_parse: out must not be NULL!");

    struct json_error error;
    return raku_json_parse_err(src, out, &error);
}

RAKU_API
enum raku_status raku_json_parse_err(const char *src, struct json_value **out, struct json_error *err)
{
    ASSERT(src != NULL,
           "raku_json_parse_err: src must not be NULL!");
    ASSERT(out != NULL,
           "raku_json_parse_err: out must not be NULL!");
    ASSERT(err != NULL,
           "raku_json_parse_err: err must not be NULL!");

    struct json_parser parser;
    json_parser_init(&parser, src);

    struct json_value *value;
    enum raku_status status = parse_value(&parser, &value);
    if (status == RAKU_OK)
    {
        skip_whitespaces(&parser.lexer);
        if (!at_end(&parser.lexer))
        {
            status = RAKU_JSON_EXPECTED_END;
            raku_json_value_free(value);
        }

        else
            *out = value;
    }

    if (status != RAKU_OK)
        *err = ERROR(parser.lexer.column, parser.lexer.row);
    
    return status;
}