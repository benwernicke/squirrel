#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>

typedef enum {
    SRC_FILE,
    SRC_HEADER,
    SRC_OTHER,
} src_type_t;

typedef struct src_t src_t;
struct src_t {
    src_type_t type;
    char identifier[128];
    char file_name[128];
    char url[512];
    char compile_flags[512];
};

src_t* get_src(uint64_t* len);
char* get_default_main(void);
char* get_default_cbuild(void);

#endif
