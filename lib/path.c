#include "path.h"

int path_cat_(char** buf, uint64_t* buf_len, ...)
{
    va_list ap;
    va_start(ap, buf_len);

    char* next_arg = va_arg(ap, char*);
    uint64_t curr_len = strlen(next_arg);
    if (curr_len >= *buf_len) {
        *buf_len += (curr_len << 1);
        *buf = realloc(*buf, *buf_len);
        if (buf == NULL) {
            return -1;
        }
    }
    strcpy(*buf, next_arg);

    bool add_slash = 0;
    while ((next_arg = va_arg(ap, char*)) != NULL) {
        add_slash = 0;
        if ((*buf)[curr_len - 1] != '/') {
            add_slash = 1;
        }
        curr_len += strlen(next_arg) + add_slash;
        if (curr_len >= *buf_len) {
            *buf_len += (curr_len << 1);
            *buf = realloc(*buf, *buf_len);
            if (*buf == NULL) {
                return -1;
            }
        }
        if (add_slash) {
            strcat(*buf, "/");
        }
        strcat(*buf, next_arg);
    }
    va_end(ap);
    return 0;
}
