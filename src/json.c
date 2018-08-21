/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#include "cel/json.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/file.h"
#include "cel/log.h"

int cel_json_init(CelJson *json)
{
    json->escape_cursor = 0;
    json->is_escape = 0;
    json->colon = 0;
    json->root_node = NULL;
    json->node_depth = 0;
    json->cur_node = NULL;
    json->state = CEL_JSONDS_ROOT;
    cel_vstring_init_a(&(json->vstr));
    cel_vstring_resize_a(&(json->vstr), 32);

    return 0;
}

int cel_json_init_buffer(CelJson *json, char *buf, size_t size)
{
    if (cel_json_init(json) != -1)
    {
        cel_json_deserialize_starts(json);
        if (cel_json_deserialize_update(json, buf, size) != -1
            || json->state != CEL_JSONDS_ERROR)
        {
            cel_json_deserialize_finish(json);
            return 0;
        }
        cel_json_deserialize_finish(json);
        cel_json_destroy(json);
    }
    CEL_ERR((_T("Json init by buffer error.(%s)"), 
        cel_geterrstr(cel_sys_geterrno())));

    return -1;
}

int cel_json_init_file(CelJson *json, const char *file)
{
    if (cel_json_init(json) != -1)
    {
        if (cel_json_deserialize_file(json, file) != -1) 
            return 0;
        cel_json_destroy(json);
    }
    CEL_ERR((_T("Json init by file error.(%s)"),
        cel_geterrstr(cel_sys_geterrno())));

    return -1;
}

void cel_json_destroy(CelJson *json)
{
    cel_json_clear(json);
    cel_vstring_destroy_a(&(json->vstr));
    json->escape_cursor = 0;
    json->is_escape = 0;
    json->colon = 0;
    json->root_node = NULL;
    json->cur_node = NULL;
    json->state = CEL_JSONDS_ROOT;
}

CelJson *cel_json_new(void)
{
    CelJson *json;

    if ((json = (CelJson *)cel_malloc(sizeof(CelJson))) != NULL)
    {
        if (cel_json_init(json) == 0)
            return json;
        cel_free(json);
    }
    return NULL;
}

CelJson *cel_json_new_buffer(char *buf, size_t size)
{
    CelJson *json;

    if ((json = (CelJson *)cel_malloc(sizeof(CelJson))) != NULL)
    {
        if (cel_json_init_buffer(json, buf, size) == 0)
            return json;
        cel_free(json);
    }
    return NULL;
}

CelJson *cel_json_new_file(const char *file)
{
    CelJson *json;

    if ((json = (CelJson *)cel_malloc(sizeof(CelJson))) != NULL)
    {
        if (cel_json_init_file(json, file) == 0)
            return json;
        cel_free(json);
    }
    return NULL;
}

void cel_json_free(CelJson *json)
{
    cel_json_destroy(json);
    cel_free(json);
}

static int cel_json_string_encode(CelJson *json, char ch)
{
    switch (ch)
    { 
    case '"': 
    case '\\': 
    case '/': 
        json->is_escape = 1; break;
    case '\b': ch = 'b'; json->is_escape = 1; break; 
    case '\f': ch = 'f'; json->is_escape = 1; break; 
    case '\n': ch = 'n'; json->is_escape = 1; break; 
    case '\r': ch = 'r'; json->is_escape = 1; break; 
    case '\t': ch = 't'; json->is_escape = 1; break; 
    default: json->is_escape = 0; break;
    }
    if (json->is_escape == 1)
    {
        //_puttchar(ch);
        cel_vstring_append_a(&(json->vstr), '\\');
        json->is_escape = 0;
    }
    cel_vstring_append_a(&(json->vstr), ch);

    return 0;
}

static int cel_json_string_decode(CelJson *json, char ch)
{
    //_tprintf(_T("%c escape %d\r\n"), ch, json->is_escape);
    if ((--(json->is_escape)) < 0)
    {
        if (ch == '\\')
        {
            json->is_escape = 1;
        }
        else 
        {
            if (ch == '"')
            {
                json->colon = 1;
                return 0;
            }
            cel_vstring_append_a(&(json->vstr), ch);
        }
    }
    else
    {
        if (json->is_escape == 0)
        {
            if (ch == 'u')
            {
                json->is_escape = 5;
                json->escape_cursor = 0;
            }
            else
            {
                switch (ch)
                { 
                case '\"':
                case '/': 
                case '\\': break;
                case 'b': ch = '\b'; break; 
                case 'f': ch = '\f'; break; 
                case 'n': ch = '\n'; break; 
                case 'r': ch = '\r'; break; 
                case 't': ch = '\t'; break; 
                default: 
                    CEL_ERR((_T("Json string decode error, undefined escap(%c)."), 
                        ch));
                    json->state = CEL_JSONDS_ERROR;
                    return -1;
                }
                //_puttchar(ch);
                cel_vstring_append_a(&(json->vstr), ch);
            }
        }
        /* Unicode */
        else
        {
            json->escape_buf[(json->escape_cursor)++] = ch;
            if (json->escape_cursor >= 4)
            {
#ifdef UNICODE
                //cel_vstring_append_a(&(json->vstr), 0);
#else
                //cel_vstring_append_a(&(json->vstr), 0);
                //cel_vstring_append_a(&(json->vstr), 0);
#endif
                json->escape_cursor = 0;
                json->is_escape = 0;
            }
        }
    }

    return 0;
}

static int cel_json_deserialize_key(CelJson *json, char *buf, size_t size,
                                    size_t *cursor)
{
    char ch;
    CelJsonNode *cur_node = json->cur_node;

    //_tprintf(_T("enter key(%d):%s\r\n"), json->vstr.size, &buf[*cursor]);
    ch = buf[*cursor];
    if (cel_vstring_len(&(json->vstr)) == 0)
    {
        while (CEL_ISSPACE_A(ch) || ch == '"')
        {
            if ((++(*cursor)) >= size) 
                return -1;
            ch = buf[*cursor];
        }
    }
    do
    {
        if (json->colon == 0)
        {
            if (cel_json_string_decode(json, ch) == -1)
                return -1;
        }
        if ((++*cursor) >= size) return -1;
    }while ((ch = buf[*cursor]) != ':');
    /* Remove spaces */
    cel_vstring_rtrim_a(&(json->vstr));
    /* Dup key string */
    if ((cur_node->key_len = cel_vstring_len(&(json->vstr))) > 0)
    {
        if ((cur_node->key = 
            (char *)cel_strdup(cel_vstring_str_a(&(json->vstr)))) != NULL)
        {
            json->colon = 0;
            cel_vstring_clear(&(json->vstr));
            //_tprintf(_T("key: %s , len %d\r\n"), cur_node->key, cur_node->key_len);
            json->state = CEL_JSONDS_VALUE;
            return 0;
        }
    }
    CEL_ERR((_T("Json deserialize key '%c' error, parent key '%s'."),
        buf[*cursor], cur_node->parent->key));
    json->state = CEL_JSONDS_ERROR;
    return -1;
}

static int cel_json_deserialize_string(CelJson *json, 
                                       char *buf, size_t size, size_t *cursor)
{
    char ch;
    CelJsonNode *cur_node = json->cur_node;

    //puts(&buf[*cursor]);
    while (TRUE)
    {
        ch = buf[*cursor];
        if (json->colon == 0)
        {
            if (cel_json_string_decode(json, ch) == -1)
                return -1;
        }
        //_tprintf(_T("colon %d, ch %c\r\n"), json->colon, ch);
        if (json->colon == 1 && ch != '"'
           /* && (ch == ',' || ch == ']' || ch == '}')*/)
            break;
        if ((++(*cursor)) >= size)
            return -1;
    }
    /* Dup value string */
    if ((cur_node->strvalue_len = cel_vstring_len(&(json->vstr))) >= 0)
    {
        if (cur_node->strvalue_len > 0)
            cur_node->strvalue = 
            (char *)cel_strdup(cel_vstring_str(&(json->vstr)));
        else
            cur_node->strvalue = NULL;
        json->colon = 0;
        cel_vstring_clear(&(json->vstr));
        json->state = CEL_JSONDS_NEXT;
        /*_tprintf(_T("key %s, value:%s, len %d\r\n"),
            cur_node->key, cur_node->strvalue, cur_node->strvalue_len);*/
        return 0;
    }
    CEL_ERR((_T("Json deserialize key '%s' value error, len %d."), 
        cur_node->key, cur_node->strvalue_len));
    json->state = CEL_JSONDS_ERROR;

    return -1;
}

static int cel_json_deserialize_number(CelJson *json,
                                       char *buf, size_t size, size_t *cursor)
{
    BOOL is_double = FALSE;
    char ch, *str;
    CelJsonNode *cur_node = json->cur_node;

    while (((ch = buf[*cursor]) != ',') && ch != ']' && ch != '}')
    {
        if (!is_double
            && ((ch = buf[*cursor]) == '.' || ch == 'e' || ch == 'E'))
        {
            is_double = TRUE;
        }
        cel_vstring_append_a(&(json->vstr), ch);
        if ((++(*cursor)) >= size)
            return -1;
    }
    /* Convert */
    str = cel_vstring_str(&(json->vstr));
    if (is_double)
    {
        cur_node->type = CEL_JSONT_DOUBLE;
        cur_node->dvalue = atof(str);
    }
    else
    {
        cur_node->type = CEL_JSONT_INT;
        cur_node->lvalue = atol(str);
    }
    //_tprintf(_T("value:%s\r\n"), cel_vstring_str(&(json->vstr)));
    cel_vstring_clear(&(json->vstr));
    json->state = CEL_JSONDS_NEXT;

    return 0;
}

static int cel_json_deserialize_value(CelJson *json,
                                      char *buf, size_t size, size_t *cursor)
{
    char ch;

    if (json->cur_node->type == CEL_JSONT_UNDEFINED)
    {
        while ((ch = buf[*cursor]) == ':' || CEL_ISSPACE_A(ch))
        {
            if ((++(*cursor)) >= size)
                return -1;
        }
        switch (ch)
        {
        case '{':
            json->cur_node->type = CEL_JSONT_OBJECT;
            json->state = CEL_JSONDS_OBJECT;
            if ((++(*cursor)) >= size)
                return -1;
            return 0;
        case '[':
            json->cur_node->type = CEL_JSONT_ARRAY;
            json->state = CEL_JSONDS_OBJECT;
            if ((++(*cursor)) >= size)
                return -1;
            return 0;
        case '"':
            json->cur_node->type = CEL_JSONT_STRING;
            if ((++(*cursor)) >= size)
                return -1;
            return cel_json_deserialize_string(json, buf, size, cursor);
        case 't':
        case 'T':
            json->cur_node->type = CEL_JSONT_BOOL;
            json->cur_node->bvalue = TRUE;
            break;
        case 'f':
        case 'F':
            json->cur_node->type = CEL_JSONT_BOOL;
            json->cur_node->bvalue = FALSE;
            break;
        case 'n':
        case 'N':
            json->cur_node->type = CEL_JSONT_NULL;
            break;
        default:
            if (CEL_ISDIGIT_A(ch) || ch == '-' || ch == '.')
            {
                json->cur_node->type = CEL_JSONT_INT;
                return cel_json_deserialize_number(json, buf, size, cursor);
            }
            //json->state = CEL_JSONDS_ERROR;
            CEL_ERR((_T("Json deserialize key '%s' value '%c' type invaild."), 
                json->cur_node->key, ch));
            return -1;
        }
    }
    switch (json->cur_node->type)
    {
    case CEL_JSONT_STRING:
        return cel_json_deserialize_string(json, buf, size, cursor);
    case CEL_JSONT_INT:
    case CEL_JSONT_DOUBLE:
        return cel_json_deserialize_number(json, buf, size, cursor);
    default:
        break;
    }
    do 
    {
        if ((++*cursor) >= size)
            return -1;
    }while (((ch = buf[*cursor]) != ',') && ch != ']' && ch != '}');
    json->state = CEL_JSONDS_NEXT;

    return 0;
}

static int cel_json_deserialize_next(CelJson *json, char *buf, size_t size,
                                     size_t *cursor)
{
    char ch;
    CelJsonNode *cur_node = json->cur_node;
    CelJsonNode *parent_node = cur_node->parent, *new_node;

    //_tprintf(_T("enter next:%s\r\n"), &buf[*cursor]);
    if (parent_node == NULL) 
        return -1;
    while ((ch = buf[*cursor]) == ','
        || ch == '}' || ch == ']'
        || CEL_ISSPACE_A(ch))
    {
        if (ch == '}' || ch == ']')
        {
            if ((cur_node = cur_node->parent) == NULL
                || (parent_node = cur_node->parent) == NULL)
            {
                json->state = CEL_JSONDS_OK;
                //puts("oK");
                return 0;
            }
            json->cur_node = cur_node;
            //_tprintf(_T("Parent node %p\r\n"), json->cur_node->parent);
        }
        if ((++(*cursor)) >= size)
        {
            //_tprintf(_T("%d - %d\r\n"), (int)*cursor, (int)size);
            return -1;
        }
    }
    //_tprintf(_T("next object:%current, parent (%p) \r\n"), buf[*cursor], parent_node);
    if ((new_node = (CelJsonNode *)cel_calloc(1, sizeof(CelJsonNode))) != NULL)
    {
        cur_node->next = new_node;
        new_node->parent = cur_node->parent;
        cur_node->parent->child_size++;
        json->cur_node = new_node;
        json->state = (parent_node->type == CEL_JSONT_ARRAY 
            ? CEL_JSONDS_VALUE : CEL_JSONDS_KEY);
        return 0;
    }

    return -1;
}

static int cel_json_deserialize_object(CelJson *json, char *buf, size_t size,
                                       size_t *cursor)
{
    char ch;
    CelJsonNode *new_node;

    while (CEL_ISSPACE_A(buf[*cursor]))
    {
        if ((++(*cursor)) >= size)
            return -1;
    }
    if ((ch = buf[*cursor]) == '}' || ch == ']')
    {
        json->state = CEL_JSONDS_NEXT;
        if ((++(*cursor)) >= size)
            return -1;
        return 0;
    }
    else if ((new_node = 
        (CelJsonNode *)cel_calloc(1, sizeof(CelJsonNode))) != NULL)
    {
        new_node->parent = json->cur_node;
        json->cur_node->child_size++;
        json->cur_node->child = new_node;
        json->cur_node = new_node;
        json->state = (new_node->parent->type == CEL_JSONT_OBJECT 
            ? CEL_JSONDS_KEY : CEL_JSONDS_VALUE);
        return 0;
    }
    json->state = CEL_JSONDS_ERROR;

    return -1;
}

static int cel_json_deserialize_root(CelJson *json, char *buf, size_t size,
                                     size_t *cursor)
{
    char ch;
    
    while (CEL_ISSPACE_A((ch = buf[*cursor])))
        if ((++(*cursor)) >= size) 
            return -1;
    json->root_node = (CelJsonNode *)cel_calloc(1, sizeof(CelJsonNode));
    json->cur_node = json->root_node;
    json->state = CEL_JSONDS_VALUE;
    //_tprintf(_T("root(%p):%s\r\n"),json->root_node, &buf[*cursor]);

    return 0;
}

static __inline void _cel_json_serialize_indent(CelVStringA *str, int depth)
{
    _cel_vstring_nappend_a((str), CEL_CRLF_A, CEL_CRLFLEN); 
    while (--(depth) >= 0)
        _cel_vstring_nappend_a((str), CEL_INDENT, CEL_INDENTLEN);
}
#define cel_json_serialize_indent(json) \
    if ((json)->indent == 1) \
        _cel_json_serialize_indent(&(json->vstr), (json)->node_depth);

static int cel_json_serialize_copy_buffer(CelJson *json, 
                                          char *buf, size_t size, 
                                          size_t *cursor)
{
    size_t len = cel_vstring_len(&(json->vstr)) - json->colon;
    char *str = cel_vstring_str(&(json->vstr));

    if (size - *cursor < len)
        len = size - *cursor;
    memcpy(buf + *cursor, &(str[json->colon]), len * sizeof(char));
    json->colon += len;
    //_tprintf("cursor = %d, colon = %d, len = %d\r\n", *cursor, json->colon, len);
    if ((*cursor += len) == size)
        return -1;
    json->colon = 0;
    cel_vstring_clear(&(json->vstr));

    return 0;
}

static int cel_json_serialize_key(CelJson *json, char *buf, size_t size,
                                  size_t *cursor)
{
    int i = 0;
    CelJsonNode *cur_node = json->cur_node;

    //_tprintf(_T("key %s, len = %d\r\n"), cur_node->key, cur_node->key_len);
    cel_vstring_append_a(&(json->vstr), '\"');
    while (i < (int)cur_node->key_len)
        cel_json_string_encode(json, cur_node->key[i++]);
    _cel_vstring_nappend_a(&(json->vstr), "\":", 2);
    json->state = CEL_JSONSS_VALUE;

    return cel_json_serialize_copy_buffer(json, buf, size, cursor);
}

static int cel_json_serialize_next(CelJson *json, char *buf, size_t size,
                                   size_t *cursor)
{
    CelJsonNode *cur_node = json->cur_node, *parent_node;
    
    while ((json->cur_node = cur_node->next) == NULL)
    {
        json->node_depth--;
        cel_json_serialize_indent(json);
        cel_vstring_append_a(&(json->vstr), 
            ((parent_node = cur_node->parent) == NULL
            || parent_node->type == CEL_JSONT_OBJECT) ? '}' : ']');
        if ((json->cur_node = parent_node) == NULL || json->node_depth == 0)
        {
            //_putts("Serialize ok!");
            _cel_vstring_nappend_a(&(json->vstr), CEL_CRLF_A, CEL_CRLFLEN); 
            json->state = CEL_JSONSS_OK;
            return cel_json_serialize_copy_buffer(json, buf, size, cursor);
        }
        cur_node = json->cur_node;
    }
    cel_vstring_append_a(&(json->vstr), ',');
    cel_json_serialize_indent(json);
    json->state = (((parent_node = json->cur_node->parent) == NULL
        || parent_node->type == CEL_JSONT_OBJECT)
        ?  CEL_JSONSS_KEY : CEL_JSONSS_VALUE);

    return 0;
}

static int cel_json_serialize_value(CelJson *json, char *buf, size_t size,
                                    size_t *cursor)
{
    int i;
    CelJsonNode *cur_node = json->cur_node;
    char num_buf[32];

    switch (cur_node->type)
    {
    case CEL_JSONT_OBJECT:
        cel_vstring_append_a(&(json->vstr), '{');
        if (cur_node->child == NULL)
        {
            cel_json_serialize_indent(json);
            cel_vstring_append_a(&(json->vstr), '}');
            return cel_json_serialize_next(json, buf, size, cursor);
        }
        else
        {
            json->node_depth++;
            json->cur_node = cur_node->child;
            cel_json_serialize_indent(json);
            return cel_json_serialize_key(json, buf, size, cursor);
        }
        break;
    case CEL_JSONT_ARRAY:
        cel_vstring_append_a(&(json->vstr), '[');
        if (cur_node->child == NULL)
        {
            cel_json_serialize_indent(json);
            cel_vstring_append_a(&(json->vstr), ']');
            return cel_json_serialize_next(json, buf, size, cursor);
        }
        else
        {
            json->node_depth++;
            json->cur_node = cur_node->child;
            cel_json_serialize_indent(json);
            return cel_json_serialize_value(json, buf, size, cursor);
        }
        break;
    case CEL_JSONT_STRING:
        cel_vstring_append_a(&(json->vstr), '\"');
        i = 0;
        while (i < (int)cur_node->strvalue_len)
            cel_json_string_encode(json, cur_node->strvalue[i++]);
        cel_vstring_append_a(&(json->vstr), '\"');
        json->state = CEL_JSONSS_NEXT;
        return cel_json_serialize_copy_buffer(json, buf, size, cursor);
    case CEL_JSONT_INT:
        i = snprintf(num_buf, 32, "%ld", cur_node->lvalue);
        _cel_vstring_nappend_a(&(json->vstr), num_buf, i);
        return cel_json_serialize_next(json, buf, size, cursor);
    case CEL_JSONT_BOOL:
        if (json->cur_node->bvalue)
            _cel_vstring_nappend_a(&(json->vstr), "true", 4);
        else
            _cel_vstring_nappend_a(&(json->vstr), "false", 5);
        return cel_json_serialize_next(json, buf, size, cursor);
    case CEL_JSONT_DOUBLE:
        i = snprintf(num_buf, 32, "%g", cur_node->dvalue);
        _cel_vstring_nappend_a(&(json->vstr), num_buf, i);
        return cel_json_serialize_next(json, buf, size, cursor);
    case CEL_JSONT_NULL:
        _cel_vstring_nappend_a(&(json->vstr), "null", 4);
        return cel_json_serialize_next(json, buf, size, cursor);
    default:
        CEL_ERR((_T("Json type %d undefined."), cur_node->type));
        json->state = CEL_JSONSS_ERROR;
        return -1;
    }

    return 0;
}

static int cel_json_serialize_root(CelJson *json, char *buf, size_t size,
                                   size_t *cursor)
{
    if (json->cur_node == NULL)
        json->state = CEL_JSONSS_OK;
    else
        json->state = CEL_JSONSS_VALUE;
    return 0;
}

typedef struct _CelJsonMachine
{
    int state;
    int (* callback) (CelJson *json, char *buf, size_t size, size_t *cursor);
}CelJsonMachine;

static CelJsonMachine s_jmtb[] = 
{
    { CEL_JSONDS_ROOT, cel_json_deserialize_root },
    { CEL_JSONDS_OBJECT, cel_json_deserialize_object },
    { CEL_JSONDS_NEXT, cel_json_deserialize_next },
    { CEL_JSONDS_KEY, cel_json_deserialize_key },
    { CEL_JSONDS_VALUE, cel_json_deserialize_value },
    { CEL_JSONDS_OK, NULL },
    { CEL_JSONDS_ERROR, NULL },
    { CEL_JSONSS_ERROR, NULL },
    { CEL_JSONSS_ROOT, cel_json_serialize_root },
    { CEL_JSONSS_NEXT, cel_json_serialize_next },
    { CEL_JSONSS_KEY, cel_json_serialize_key },
    { CEL_JSONSS_VALUE, cel_json_serialize_value },
    { CEL_JSONSS_OK, NULL }
};

int cel_json_deserialize_starts(CelJson *json)
{
    json->state = CEL_JSONDS_ROOT;
    json->colon = 0;
    cel_vstring_clear(&(json->vstr));
    json->node_depth = 0;
    json->cur_node = json->root_node;

    return 0;
}

int cel_json_deserialize_update(CelJson *json, char *buf, size_t size)
{
    size_t cursor = 0;

    if (json->state >= CEL_JSONDS_ERROR)
    {
        CEL_ERR((_T("Json deserialize update failed, state %d error."),
            json->state));
        return -1;
    }
    while (json->state != CEL_JSONDS_OK)
    {
        if (s_jmtb[json->state].callback(json, buf, size, &cursor) == -1)
        {
            if (json->state == CEL_JSONDS_ERROR)
                return -1;
            break;
        }
    }

    return 0;
}

int cel_json_deserialize_finish(CelJson *json)
{
    if (json->state != CEL_JSONDS_OK)
    {
        if (json->state != CEL_JSONDS_ERROR)
        {
            if (json->cur_node != NULL)
                CEL_ERR((_T("Json deserialize state %d, cur node key '%s'."), 
                json->state, json->cur_node->key));
            else
                CEL_ERR((_T("Json deserialize state %d."), json->state));
        }
    }
    return 0;
}

int cel_json_deserialize_file(CelJson *json, const char *file)
{
    FILE *fp;
    char lbuf[CEL_JSON_LINE_SIZE];

    if ((fp = fopen(file, "rb")) != NULL)
    {
        cel_json_deserialize_starts(json);
        while (fgets(lbuf, CEL_JSON_LINE_SIZE, fp) != NULL)
        {
            if (cel_json_deserialize_update(json, lbuf, strlen(lbuf)) == -1
                && json->state == CEL_JSONDS_ERROR)
            {
                cel_json_deserialize_finish(json);
                cel_fclose(fp);
                return -1;
            }
        }
        cel_fclose(fp);
        cel_json_deserialize_finish(json);
        if (json->state == CEL_JSONDS_OK)
            return 0;
    }
    return -1;
}

int cel_json_serialize_starts(CelJson *json, int indent)
{
    json->indent = indent;
    json->colon = 0;
    cel_vstring_clear(&(json->vstr));
    json->node_depth = 0;
    json->cur_node = json->root_node;
    json->state = CEL_JSONSS_ROOT;

    return 0;
}

int cel_json_serialize_update(CelJson *json, char *buf, size_t *size)
{
    size_t cursor = 0;

    if (json->state < CEL_JSONSS_ROOT)
    {
        CEL_ERR((_T("Json serialize update failed, state %d error."),
            json->state));
        return -1;
    }
    if (json->colon == 0
        || cel_json_serialize_copy_buffer(json, buf, *size, &cursor) != -1)
    {
        while (json->state != CEL_JSONSS_OK)
        {
            if (s_jmtb[json->state].callback(json, buf, *size, &cursor) == -1)
            {
                if (json->state == CEL_JSONSS_ERROR)
                    return -1;
                break;
            }
        }
    }
    *size = cursor;
    return 0;
}

int cel_json_serialize_finish(CelJson *json)
{
    json->state = CEL_JSONSS_OK;
    json->indent = 0;
    json->colon = 0;
    cel_vstring_clear(&(json->vstr));
    json->node_depth = 0;
    json->cur_node = json->root_node;

    return 0;
}

int cel_json_serialize_file(CelJson *json, const char *file, int indent)
{
    FILE *fp;
    size_t size;
    char lbuf[CEL_JSON_LINE_SIZE];

   if ((fp = fopen(file, "wb")) != NULL)
    {
        cel_json_serialize_starts(json, indent);
        do
        {
            size = CEL_JSON_LINE_SIZE - 1;
            if (cel_json_serialize_update(json, lbuf, &size) == -1
                && json->state == CEL_JSONSS_ERROR)
            {
                cel_json_serialize_finish(json);
                cel_fclose(fp);
                return -1;
            }
            lbuf[size] = '\0';
            //printf("%s", lbuf);
        }while (fwrite(lbuf, 1, size, fp) != EOF 
            && (json->state != CEL_JSONSS_OK || json->colon != 0));
        cel_fclose(fp);
        if (json->state == CEL_JSONSS_OK)
        {
            cel_json_serialize_finish(json);
            return 0;
        }
    }
    return -1;
}

CelJsonNode *cel_json_node_new(CelJsonType type)
{
    CelJsonNode *node;
    if ((node = (CelJsonNode *)cel_calloc(1, sizeof(CelJsonNode))) != NULL)
        node->type = type;
    return node;
}

void cel_json_node_free(CelJsonNode *node)
{
    CelJsonNode *next_node, *child_node;

    //_tprintf(_T("start free key %s\r\n"), node->key);
    switch (node->type)
    {
    case CEL_JSONT_STRING:
        if (node->strvalue != NULL)
            cel_free(node->strvalue);
        break;
    case CEL_JSONT_OBJECT:
    case CEL_JSONT_ARRAY:
        child_node = node->child;
        while (child_node != NULL)
        {
            next_node = child_node->next;
            cel_json_node_free(child_node);
            child_node = next_node;
        }
    default:
        break;
    }
    //_tprintf(_T("end free key %s\r\n"), node->key);
    if (node->key != NULL)
        cel_free(node->key);
    cel_free(node);
}

CelJsonNode *cel_json_bool_new(BOOL bool_value)
{
    CelJsonNode *node;
    if ((node = cel_json_node_new(CEL_JSONT_BOOL)) != NULL)
        node->bvalue = bool_value;
    return node;
}

CelJsonNode *cel_json_long_new(long int_value)
{
    CelJsonNode *node;
    if ((node = cel_json_node_new(CEL_JSONT_INT)) != NULL)
        node->lvalue = int_value;
    return node;
}

CelJsonNode *cel_json_double_new(double double_value)
{
    CelJsonNode *node;
    if ((node = cel_json_node_new(CEL_JSONT_DOUBLE)) != NULL) 
        node->dvalue = double_value;
    return node;
}

CelJsonNode *cel_json_string_new(const char *string_value)
{
    CelJsonNode *node;

    if (string_value != NULL)
    {
        if ((node = cel_json_node_new(CEL_JSONT_STRING)) != NULL)
        {
            node->strvalue = cel_strdup(string_value); 
            node->strvalue_len = strlen(string_value);
            return node;
        }
    }
    return NULL;
}

void cel_json_array_add(CelJsonNode *_array, CelJsonNode *item)
{
    CelJsonNode *current;

    if (item != NULL) 
    {
        if ((current = _array->child) == NULL) 
        { 
            _array->child = item;
        } 
        else 
        {
            while (current != NULL && current->next != NULL) 
                current = current->next; 
            current->next = item;
        }
        item->parent = _array;
        _array->child_size++;
    }
}

void cel_json_object_add(CelJsonNode *object,
                         const char *key, CelJsonNode *item)
{
    CelJsonNode *current;

    if (item != NULL)
    {
        if (key != NULL)
        {
            item->key = cel_strdup(key); 
            item->key_len = strlen(key);
        }
        if ((current = object->child) == NULL) 
        { 
            object->child = item;
        } 
        else 
        {
            while (current != NULL && current->next != NULL) 
                current = current->next;
            current->next = item;
        }
        item->parent = object;
        object->child_size++;
    }
}

CelJsonNode *cel_json_array_detach(CelJsonNode *_array, int which)
{
    CelJsonNode *previous = NULL, *current = _array->child;

    while (current != NULL && which-- > 0)
    {
        if (which-- == 0)
        {
            if (previous != NULL) 
                previous->next = current->next;
            if (current == _array->child) 
                _array->child = current->next;
            _array->child_size--;
            return current;
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}

void cel_json_array_delete(CelJsonNode *_array, int which)
{
    CelJsonNode *node;
    if ((node = cel_json_array_detach(_array, which)) != NULL)
        cel_json_node_free(node);
}

CelJsonNode *cel_json_object_detach(CelJsonNode *object, const char *key)
{
    CelJsonNode *previous = NULL, *current = object->child;

    while (current != NULL) 
    {
        if (strcmp(current->key, key) == 0)
        {
            if (previous != NULL)
                previous->next = current->next;
            if (current == object->child)
                object->child = current->next;
            object->child_size--;
            return current;
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}

void cel_json_object_delete(CelJsonNode *object, const char *key)
{
    CelJsonNode *node;
    if ((node = cel_json_object_detach(object, key)) != NULL)
        cel_json_node_free(node);
}

int cel_json_array_repalce(CelJsonNode *_array, int which, CelJsonNode *item)
{
    CelJsonNode *previous = NULL, *current = _array->child;

    while (current != NULL)
    {
        if (which-- == 0)
        {
            if (previous != NULL)
                previous->next = item;
            else if (current == _array->child)
                _array->child = item;
            item->next = current->next;
            item->parent = _array;
            cel_json_node_free(current);
            return 0;
        }
        previous = current;
        current = current->next;
    }
    return -1;
}

int cel_json_object_replace(CelJsonNode *object,
                            const char *key, CelJsonNode *item)
{
    CelJsonNode *previous = NULL, *current = object->child;

    while (current != NULL) 
    {
        if (strcmp(current->key, key) == 0)
        {
            item->key = current->key;
            item->key_len = current->key_len;
            current->key = NULL;
            current->key_len = 0;
            if (previous != NULL)
                previous->next = item;
            else if (current == object->child)
                object->child = item;
            item->next = current->next;
            item->parent = object;
            cel_json_node_free(current);
            return 0;
        }
        previous = current;
        current = current->next;
    }

    return -1;
}

CelJsonNode *cel_json_array_get(CelJsonNode *_array, int which)
{
    CelJsonNode *current; 

    if (_array->type != CEL_JSONT_ARRAY) 
        return NULL;
    current = _array->child;
    while (current != NULL && which-- > 0) 
        current = current->next; 
    return current;
}

int cel_json_array_get_bool(CelJsonNode *_array, int which, BOOL *value)
{
    CelJsonNode *node;

    if ((node = cel_json_array_get(_array, which)) != NULL
        && node->type == CEL_JSONT_BOOL)
    {
        *value = node->bvalue;
        return 0;
    }
    return -1;
}

int cel_json_array_get_int(CelJsonNode *_array, int which, int *value)
{
    CelJsonNode *node;

    if ((node = cel_json_array_get(_array, which)) != NULL
        && node->type == CEL_JSONT_INT)
    {
        *value = node->lvalue;
        return 0;
    }
    return -1;
}

int cel_json_array_get_long(CelJsonNode *_array, int which, long *value)
{
    CelJsonNode *node;

    if ((node = cel_json_array_get(_array, which)) != NULL
        && node->type == CEL_JSONT_INT)
    {
        *value = node->lvalue;
        return 0;
    }
    return -1;
}

int cel_json_array_get_double(CelJsonNode *_array, int which, double *value)
{
    CelJsonNode *node;

    if ((node = cel_json_array_get(_array, which)) != NULL 
        && node->type == CEL_JSONT_DOUBLE)
    {
        *value = node->dvalue;
        return 0;
    }
    return -1;
}

int cel_json_array_get_string(CelJsonNode *_array, 
                              int which, char *value, size_t size)
{
    CelJsonNode *node;

    if ((node = cel_json_array_get(_array, which)) != NULL 
        && node->type == CEL_JSONT_STRING)
    {
        if (size > node->strvalue_len)
            size = node->strvalue_len + 1;
        memcpy(value, node->strvalue, size * sizeof(char));
        value[size - 1] = '\0';
        return 0;
    }
    return -1;
}

int cel_json_array_get_string_alloc(CelJsonNode *_array, 
                                    int which, char **value)
{
    size_t size = 1024;
    CelJsonNode *node;

    if ((node = cel_json_array_get(_array, which)) != NULL 
        && node->type == CEL_JSONT_STRING)
    {
        if (size > node->strvalue_len)
            size = node->strvalue_len + 1;
        if ((*value = (char *)cel_malloc(size * sizeof(char))) != NULL)
        {
            memcpy(*value, node->strvalue, size * sizeof(char));
            (*value)[size - 1] = '\0';
            return 0;
        }
    }
    return -1;
}

CelJsonNode *cel_json_object_get(CelJsonNode *object, const char *key)
{
    CelJsonNode *current;

    if (object->type != CEL_JSONT_OBJECT)
        return NULL;
    current = object->cursor;
    while (current != NULL)
    {
        //_tprintf(_T("***************%s\r\n"), object->key);
        if (current->key != NULL && strcmp(current->key, key) == 0)
        {
            object->cursor = current->next;
            return current;
        }
        current = current->next;
    }
    current = object->child;
    while (current != object->cursor)
    {
        //_tprintf(_T("###############%s\r\n"), object->key);
        if (current->key != NULL && strcmp(current->key, key) == 0)
        {
            object->cursor = current->next;
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int cel_json_object_get_bool(CelJsonNode *object, const char *key, BOOL *value)
{
    CelJsonNode *node;

    if ((node = cel_json_object_get(object, key)) != NULL
        && node->type == CEL_JSONT_BOOL)
    {
        *value = node->bvalue;
        return 0;
    }
    return -1;
}

int cel_json_object_get_int(CelJsonNode *object, const char *key, int *value)
{
    CelJsonNode *node;

    if ((node = cel_json_object_get(object, key)) != NULL
        && node->type == CEL_JSONT_INT)
    {
        *value = node->lvalue;
        return 0;
    }
    return -1;
}

int cel_json_object_get_long(CelJsonNode *object, const char *key, long *value)
{
    CelJsonNode *node;

    if ((node = cel_json_object_get(object, key)) != NULL
        && node->type == CEL_JSONT_INT)
    {
        *value = node->lvalue;
        return 0;
    }
    return -1;
}

int cel_json_object_get_double(CelJsonNode *object, 
                               const char *key, double *value)
{
    CelJsonNode *node;

    if ((node = cel_json_object_get(object, key)) != NULL
        && node->type == CEL_JSONT_DOUBLE)
    {
        *value = node->dvalue;
        return 0;
    }
    return -1;
}

int cel_json_object_get_string(CelJsonNode *object, const char *key, 
                               char *value, size_t size)
{
    CelJsonNode *node;

    if ((node = cel_json_object_get(object, key)) != NULL
        && node->type == CEL_JSONT_STRING)
    {
        if (size > node->strvalue_len)
            size = node->strvalue_len + 1;
        if (size > 1)
            memcpy(value, node->strvalue, size * sizeof(char));
        value[size - 1] = '\0';
        return 0;
    }
    return -1;
}

int cel_json_object_get_string_alloc(CelJsonNode *obj, 
                                     const char *key, char **value)
{
    size_t size = 1024;
    CelJsonNode *node;

    if ((node = cel_json_object_get(obj, key)) != NULL
        && node->type == CEL_JSONT_STRING)
    {
        if (size > node->strvalue_len)
            size = node->strvalue_len + 1;
         if ((*value = (char *)cel_malloc(size * sizeof(char))) != NULL)
        {
            memcpy(*value, node->strvalue, size * sizeof(char));
            (*value)[size - 1] = '\0';
            return 0;
        }
    }
    return -1;
}

int cel_json_object_get_string_pointer(CelJsonNode *obj, 
                                       const char *key, char **value)
{
    CelJsonNode *node;

    if ((node = cel_json_object_get(obj, key)) != NULL
        && node->type == CEL_JSONT_STRING)
    {
        *value = node->strvalue;
        return 0;
    }
    return -1;
}
