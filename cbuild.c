#define CBUILD
#include "cbuild.h"

#define FLAGS "-g -Wall -pedantic -lcurl"
#define FAST_FLAGS "-Ofast -march=native -lcurl"

void compile_everything(char* flags)
{
    compile_object("squirrel.c", flags, "build/squirrel.o");
    compile_object("config.c", flags, "build/config.o");
    compile_object("lib/path.c", flags, "build/path.o");
    compile_object("lib/vector.c", flags, "build/vector.o");
    compile_object_directory("main", flags, "build/");
}

void clean()
{
    printf("rm build/*\n");
    system("rm build/*");

    printf("rm -rf test/*\n");
    system("rm -r test/*");

    printf("rm main\n");
    system("rm main");
}

int main(int argc, char** argv)
{
    auto_update();

    if (argc == 1) {
        compile_everything(FLAGS);
    } else {
        if (strcmp(argv[1], "clean") == 0) {
            clean();
        } else if (strcmp(argv[1], "clean-test") == 0) {
            printf("rm -rf test/*\n");
            system("rm -rf test/*");
        } else if (strcmp(argv[1], "fast") == 0) {
            clean();
            compile_everything(FAST_FLAGS);
        } else if (strcmp(argv[1], "install-user") == 0) {
            printf("cp main ~/bin/squirrel\n");
            system("cp main ~/bin/squirrel");
        }
    }
    return 0;
}
