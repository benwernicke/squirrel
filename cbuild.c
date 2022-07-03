#define CBUILD
#include "cbuild.h"

#define FLAGS "-g -Wall -pedantic -lcurl"

void compile_everything()
{
    compile_object("squirrel.c", FLAGS, "build/squirrel.o");
    compile_object("config.c", FLAGS, "build/config.o");
    compile_object("lib/buffer.c", "-Ofast -march=native", "build/lib/buffer.o");
    compile_object_directory("main", FLAGS, "build/");
}

void clean()
{
    printf("rm build/*\n");
    system("rm build/*");

    printf("rm -rf test/*\n");
    system("rm -r test/*");
}

int main(int argc, char** argv)
{
    auto_update();
    if (argc == 1) {
        compile_everything();
    } else {
        if (strcmp(argv[1], "clean") == 0) {
            clean();
        } else if (strcmp(argv[1], "clean-test") == 0) {
            printf("rm -rf test/*\n");
            system("rm -rf test/*");
        }
    }
    return 0;
}
