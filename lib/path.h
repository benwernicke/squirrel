#ifndef PATH_H
#define PATH_H

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define path_cat(bp, lp, ...) path_cat_(bp, lp, __VA_ARGS__, NULL)
#define path_cat_or_panic(bp, lp, ...)                                                      \
    if (path_cat_(bp, lp, __VA_ARGS__, NULL) != 0) {                                  \
        fprintf(stderr, "\033[31mError: \033[39m could not concatenate path: bad_alloc\n"); \
        exit(1);                                                                            \
    }

int path_cat_(char** buf, uint64_t* buf_len, ...);

#endif
