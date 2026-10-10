#ifndef MINI_C_COMMON_H
#define MINI_C_COMMON_H

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *xmalloc(size_t n)
{
    void *p = malloc(n);
    if (p == NULL && n != 0) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

static inline void *xcalloc(size_t n, size_t size)
{
    void *p = calloc(n, size);
    if (p == NULL && n != 0 && size != 0) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

static inline void *xrealloc(void *p, size_t n)
{
    void *q = realloc(p, n);
    if (q == NULL && n != 0) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return q;
}

static inline char *xstrdup(const char *s)
{
    size_t n = strlen(s);
    char *p = xmalloc(n + 1);
    memcpy(p, s, n + 1);
    return p;
}

#endif
