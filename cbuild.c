#define CBUILD
#include "lib/cbuild.h"

#define FLAGS "-g -Wall -pedantic"
#define FAST_FLAGS " -Ofast -march=native"
#define PROFILE_FLAGS " -pg -Wall -pedantic"

void compile_all(char* flags)
{
    compile_object("build/main.o", flags, "-lcurl", "main.c");
    compile_object("build/config.o", flags, "", "config.c");
    compile_object("build/path.o", flags, "", "lib/path.c");
    compile_object("build/set.o", flags, "", "lib/set.c");
    compile_object("build/vector.o", flags, "", "lib/vector.c");
    compile_object("build/hash.o", flags, "", "lib/hash.c");
    compile_object("build/cmp.o", flags, "", "lib/cmp.c");
    compile_object_directory("out", flags, "-lcurl", "build/");
}

void clean(void)
{
    rm("build/*");
    rm("out");
    rm("test/*");
}

int main(int argc, char** argv)
{
    auto_update();
    if (argc == 1) {
        compile_all(FLAGS);
    } else if (strcmp(argv[1], "profile") == 0) {
        compile_all(PROFILE_FLAGS);
    } else if (strcmp(argv[1], "fast") == 0) {
        compile_all(FAST_FLAGS);
    } else if (strcmp(argv[1], "clean") == 0) {
        clean();
    } else if (strcmp(argv[1], "install") == 0) {
        printf("cp out /usr/local/bin/\n");
        system("cp out /usr/local/bin/");
    } else {
        fprintf(stderr, "\033[31mError: \033[39m unknown option: %s\n", argv[1]);
    }
    return 0;
}
