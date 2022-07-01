#define CBUILD
#include "cbuild.h"

#define FLAGS "-g -Wall -pedantic"

int main(void)
{
    auto_update();
    compile_object("squirrel.c", FLAGS, "build/squirrel.o");
    compile_object_directory("main", FLAGS, "build/");
    return 0;
}
