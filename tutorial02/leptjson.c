#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>    /* HUGE_VAL */
#include <errno.h>   /* ERANGE */

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')

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
    const char *l = literal, *j = c->json;
    while (*l)
        if (*j++ != *l++)
            return LEPT_PARSE_INVALID_VALUE;
    c->json = j;
    v->type = t;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
    char *end;
    const char *p = c->json;
    /* validate number */
    /* validate '-' */
    if (*p == '-') p++;

    /* validate digits before '.' */
    if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
    if (*p == '0') p++;
    else for (; ISDIGIT(*p); p++);

    /* validate '.' */
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (; ISDIGIT(*p); p++);
    }

    /* validate 'e' or 'E' */
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (; ISDIGIT(*p); p++);
    }

    /* parse number */
    errno = 0;
    v->n = strtod(c->json, &end);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    if (p != end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = p;
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
    const char *t;
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
