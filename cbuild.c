#define CBUILD
#include "cbuild.h"

#define FLAGS "-g -Wall -pedantic -lcurl"

int main(int argc, char** argv)
{
    auto_update();
    if (argc == 1) {
        compile_object("squirrel.c", FLAGS, "build/squirrel.o");
        compile_object("config.c", FLAGS, "build/config.o");
        compile_object_directory("main", FLAGS, "build/");
    } else {
        if (strcmp(argv[1], "recomp") == 0) {
            compile("build/squirrel.o", "squirrel.c", FLAGS, " -c ");
            compile("build/config.o", "config.c", FLAGS, " -c ");
            compile_object_directory("main", FLAGS, "build/");
        }
    }
    return 0;
}
