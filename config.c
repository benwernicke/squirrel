#include "type.h"

static char default_cbuild[] = "#define CBUILD\n"
                               "#include \"lib/cbuild.h\"\n"
                               "\n"
                               "#define FLAGS \"-g -Wall -pedantic\"\n"
                               "#define FAST_FLAGS \" -Ofast -march=native\"\n"
                               "#define PROFILE_FLAGS \" -pg -Wall -pedantic\"\n"
                               "\n"
                               "void compile_all(char* flags)\n"
                               "{\n"
                               "    compile_object(\"build/main.o\", flags, \"main.c\");\n"
                               "    compile_object_directory(\"out\", flags, \"build/\");\n"
                               "}\n"
                               "\n"
                               "void clean(void)\n"
                               "{\n"
                               "    system(\"rm build/*\");\n"
                               "    system(\"rm out\");\n"
                               "}\n"
                               "\n"
                               "int main(int argc, char** argv)\n"
                               "{\n"
                               "    auto_update();\n"
                               "    if(argc == 1) {\n"
                               "        compile_all(FLAGS);\n"
                               "    } else if(strcmp(argv[1], \"profile\") == 0) {\n"
                               "         compile_all(PROFILE_FLAGS);\n"
                               "    } else if(strcmp(argv[1], \"fast\") == 0) {\n"
                               "        clean();\n"
                               "        compile_all(FAST_FLAGS);\n"
                               "    } else if(strcmp(argv[1], \"clean\") == 0) {\n"
                               "        clean();\n"
                               "    } else {\n"
                               "        fprintf(stderr, \"\\033[31mError: \\033[39m unknown option: %s\\n\", argv[1]);\n"
                               "    }"
                               "\n"
                               "    return 0;\n"
                               "}\n";

static char default_main[] = "#include <stdio.h>\n"
                             "\n"
                             "int main(int argc, char** argv)\n"
                             "{\n"
                             "    printf(\"Hello World!\\n\");\n"
                             "    return 0;\n"
                             "}\n";

// note:
//       .identifier < 128
//       .file_name  < 128
//       .url        < 512
//
// if more needed: alter src_t struct in config.h

static src_t src[]
    = {
          // cmp ------------------------------------------------------------------------------------------------------------------------
          {
              .type = SRC_FILE,
              .identifier = "cmp",
              .file_name = "cmp.c",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/cmp/cmp.c",
          },
          {
              .type = SRC_HEADER,
              .identifier = "cmp",
              .file_name = "cmp.h",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/cmp/cmp.h",
          },
          // hash ------------------------------------------------------------------------------------------------------------------------
          {
              .type = SRC_FILE,
              .identifier = "hash",
              .file_name = "hash.c",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/hash/hash.c",
          },
          {
              .type = SRC_HEADER,
              .identifier = "hash",
              .file_name = "hash.h",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/hash/hash.h",
          },
          // vector ------------------------------------------------------------------------------------------------------------------------
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
          // set ------------------------------------------------------------------------------------------------------------------------
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
          // buffer ------------------------------------------------------------------------------------------------------------------------
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
          // path ------------------------------------------------------------------------------------------------------------------------
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
          // panic ------------------------------------------------------------------------------------------------------------------------
          {
              .type = SRC_HEADER,
              .identifier = "panic",
              .file_name = "panic.h",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/panic/panic.h",
          },

          // map ------------------------------------------------------------------------------------------------------------------------
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
          // sv ------------------------------------------------------------------------------------------------------------------------
          {
              .type = SRC_FILE,
              .identifier = "sv",
              .file_name = "sv.c",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/sv/sv.c",
          },
          {
              .type = SRC_HEADER,
              .identifier = "sv",
              .file_name = "sv.h",
              .url = "https://raw.githubusercontent.com/benwernicke/datastructures/main/sv/sv.h",
          },
          // asaw ------------------------------------------------------------------------------------------------------------------------
          {
              .type = SRC_FILE,
              .identifier = "asaw",
              .file_name = "asaw.c",
              .url = "https://raw.githubusercontent.com/benwernicke/asaw/main/asaw.c",
              .compile_flags = "-lpthread",
          },
          {
              .type = SRC_HEADER,
              .identifier = "asaw",
              .file_name = "asaw.h",
              .url = "https://raw.githubusercontent.com/benwernicke/asaw/main/asaw.h",
          },
      };

src_t* get_src(uint64_t* len)
{
    *len = sizeof(src) / sizeof(src_t);
    return src;
}

char* get_default_main(void)
{
    return default_main;
}

char* get_default_cbuild(void)
{
    return default_cbuild;
}
