#include "config.h"

// note:
//       .identifier < 128
//       .file_name  < 128
//       .url        < 512
//
// if more needed: alter src_t struct in config.h

static src_t src[] = {
    //vector ------------------------------------------------------------------------------------------------------------------------
    {
        .type = SRC_FILE,
        .identifier = "vector",
        .file_name = "vector.c",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/vector/vector.c",
    },
    {
        .type = SRC_HEADER,
        .identifier = "vector",
        .file_name = "vector.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/vector/vector.h",
    },
    {
        .type = SRC_HEADER,
        .identifier = "vector",
        .file_name = "ds_error.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/ds_error/ds_error.h",
    },
    //set ------------------------------------------------------------------------------------------------------------------------
    {
        .type = SRC_HEADER,
        .identifier = "set",
        .file_name = "ds_error.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/ds_error/ds_error.h",
    },
    {
        .type = SRC_HEADER,
        .identifier = "set",
        .file_name = "set.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/set/set.h",
    },
    {
        .type = SRC_FILE,
        .identifier = "set",
        .file_name = "set.c",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/set/set.c",
    },
    //buffer ------------------------------------------------------------------------------------------------------------------------
    {
        .type = SRC_FILE,
        .identifier = "buffer",
        .file_name = "buffer.c",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/buffer/buffer.c",
    },
    {
        .type = SRC_HEADER,
        .identifier = "buffer",
        .file_name = "buffer.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/buffer/buffer.h",
    },
    {
        .type = SRC_HEADER,
        .identifier = "buffer",
        .file_name = "ds_error.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/ds_error/ds_error.h",
    },
    //path ------------------------------------------------------------------------------------------------------------------------
    {
        .type = SRC_FILE,
        .identifier = "path",
        .file_name = "path.c",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/path/path.c",
    },
    {
        .type = SRC_HEADER,
        .identifier = "path",
        .file_name = "path.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/path/path.h",
    },
    {
        .type = SRC_HEADER,
        .identifier = "path",
        .file_name = "ds_error.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/ds_error/ds_error.h",
    },
    //panic ------------------------------------------------------------------------------------------------------------------------
    {
        .type = SRC_HEADER,
        .identifier = "panic",
        .file_name = "panic.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/panic/panic.h",
    },

    //map ------------------------------------------------------------------------------------------------------------------------
    {
        .type = SRC_FILE,
        .identifier = "map",
        .file_name = "map.c",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/map/map.c",
    },
    {
        .type = SRC_HEADER,
        .identifier = "map",
        .file_name = "map.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/map/map.h",
    },
    {
        .type = SRC_HEADER,
        .identifier = "map",
        .file_name = "ds_error.h",
        .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/ds_error/ds_error.h",
    },
};

src_t* get_src(uint64_t* len)
{
    *len = sizeof(src) / sizeof(src_t);
    return src;
}
