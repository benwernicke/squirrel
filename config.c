#include "config.h"

// note:
//       .identifier < 128
//       .file_name  < 128
//       .url        < 512
//
// if more needed: alter src_t struct in config.h

static src_t src[] = {
    {
        .type = SRC_FILE,
        .identifier = "vector",
        .file_name = "vector.c",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/vector.c",
        .compile_flags = "-Ofast -march=native",
    },
    {
        .type = SRC_HEADER,
        .identifier = "vector",
        .file_name = "vector.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/vector.h",
    },
    {
        .type = SRC_HEADER,
        .identifier = "vector",
        .file_name = "ds_error.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/ds_error.h",
    },
};

src_t* get_src(uint64_t* len)
{
    *len = sizeof(src) / sizeof(src_t);
    return src;
}
