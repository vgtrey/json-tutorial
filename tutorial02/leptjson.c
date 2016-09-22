#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>    /* HUGE_VAL */
#include <stdio.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define IDSIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, lept_type t, const char *literal) {
    const char *l = literal;
    while (*l)
        if (*c->json++ != *l++)
            return LEPT_PARSE_INVALID_VALUE;
    v->type = t;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
    char *end;
    const char *t;
    /* validate number */
    /* Starts with '-' or digit. */
    if (c->json[0] != '-' && !ISDIGIT(c->json[0]))
        return LEPT_PARSE_INVALID_VALUE;
    /* After zero should be '.' or nothing. */
    if (c->json[0] == '0' && c->json[1] != '\0' && c->json[1] != '.')
        return LEPT_PARSE_INVALID_VALUE;
    /* After '.' should be at least a digit. */
    for (t = c->json; *t != '\0' && *t != '.'; t++);
    if (*t == '.' && !ISDIGIT(t[1]))
        return LEPT_PARSE_INVALID_VALUE;

    /* parse number */
    v->n = strtod(c->json, &end);
    if (v->n == HUGE_VAL)
        return LEPT_PARSE_NUMBER_TOO_BIG;
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context *c, lept_value *v) {
    switch (*c->json) {
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        case 't':
            return lept_parse_literal(c, v, LEPT_TRUE, "true");
        case 'f':
            return lept_parse_literal(c, v, LEPT_FALSE, "false");
        case 'n':
            return lept_parse_literal(c, v, LEPT_NULL, "null");
        default:
            return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    int ret;
    const char* t;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        t = c.json;
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = t == c.json ? LEPT_PARSE_INVALID_VALUE : LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
