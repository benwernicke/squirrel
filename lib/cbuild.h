#ifndef CBUILD_H
#define CBUILD_H
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define GREEN "\033[32m"
#define RESET "\033[39m"
#define RED "\033[31m"

#ifndef CBUILD_BUILD_DIR
#define CBUILD_BUILD_DIR "cbuild_build"
#endif

#ifndef COMPILER

#if defined(__clang__)
#define COMPILER "clang"

#elif defined(__GNUG__)
#define COMPILER "g++"

#elif defined(__GNUC__)
#define COMPILER "gcc"

#elif defined(__MSVC_VER__)
#define COMPILER "msvc"

#elif defined(__TINYC__)
#define COMPILER "tcc"

#elif defined(__MINGW32__)
#define COMPILER "mingw"

#elif defined(__MINGW64__)
#define COMPILER "mingw"

#endif

// TODO: mingw

#endif // COMPILER

#ifndef CBUILD_EXE
#define CBUILD_EXE "cbuild"
#endif

#ifndef CBUILD_SRC
#define CBUILD_SRC "cbuild.c"
#endif

#define run_command(comp, ...) run_command_(comp, __VA_ARGS__, NULL)
void auto_update();
void compile_object(char* path, char* flags, char* obj);
#define compile_object(out, ...) print_comp_command_(COMPILER, "-c -o ", out, __VA_ARGS__, NULL), run_command(COMPILER, "-c -o", out, __VA_ARGS__, NULL)
#define compile(out, ...) print_comp_command_(COMPILER, "-o", out, __VA_ARGS__, NULL), run_command_(COMPILER, "-o", out, __VA_ARGS__, NULL)
#define rm(file)                                   \
    printf(GREEN "removing:" RESET " %s\n", file); \
    system("rm " file);

#endif
#ifdef CBUILD

static void print_comp_command_(char* comp, ...)
{
    va_list ap;
    va_start(ap, comp);
    char* cmd = malloc(256);
    uint64_t len = 256;
    uint64_t new_len = strlen(comp);
    strcpy(cmd, comp);
    char* next_arg = NULL;

    while ((next_arg = va_arg(ap, char*)) != NULL) {
        new_len += strlen(next_arg) + 3;
        if (new_len >= len) {
            cmd = realloc(cmd, new_len << 1);
            len = new_len << 1;
        }
        strcat(cmd, " ");
        strcat(cmd, next_arg);
    }
    printf(GREEN "compiling: " RESET "%s\n", cmd);

    free(cmd);
}

static int run_command_(char* comp, ...);
static long file_last_mod(char* path);

static bool file_exists(char* path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    } else {
        fclose(f);
        return 1;
    }
}

static time_t file_last_mod(char* path)
{
    struct stat s;
    stat(path, &s);
    return s.st_mtim.tv_sec;
}

void auto_update()
{
    time_t last_mod_src = file_last_mod(CBUILD_SRC);
    time_t last_mod_exe = file_last_mod(CBUILD_EXE);
    if (last_mod_src > last_mod_exe) {
        printf("\033[32mcompiling:\033[39m %s\n", CBUILD_SRC);
        if (run_command(COMPILER, "-o ", CBUILD_EXE, CBUILD_SRC) == 0) {
            char new_run[3 + strlen(CBUILD_EXE)];
            strcpy(new_run, "./");
            strcat(new_run, CBUILD_EXE);
            system(new_run);
            exit(0);
        } else {
            printf(RED "error: " RESET "while compiling %s\n", CBUILD_SRC);
        }
    }
}

static bool is_objfile_(char* name, uint64_t len)
{
    if (name[len - 2] != '.') {
        return 0;
    }
    if (name[len - 1] != 'o') {
        return 0;
    }
    return 1;
}

void compile_object_directory(char* out, char* flags, char* extra_flags, char* path)
{
    DIR* dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, RED "error: " RESET "could not open dir: %s, %s\n", path, strerror(errno));
    }
    struct dirent* entry = NULL;
    uint64_t cmd_len = strlen(COMPILER) + strlen(out) + 256 + strlen(flags) + strlen(extra_flags) + 5;
    uint64_t cmd_used = cmd_len - 256 + 5;
    char* cmd = malloc(cmd_len);
    strcpy(cmd, COMPILER);
    strcat(cmd, " -o ");
    strcat(cmd, out);
    strcat(cmd, " ");
    strcat(cmd, flags);
    strcat(cmd, " ");
    strcat(cmd, extra_flags);
    uint64_t len;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) {
            continue;
        }
        if (entry->d_type == DT_DIR) {
            fprintf(stderr, RED "warning: " RESET "possible error found: %s is directory %s is not recusive\n", entry->d_name, __func__);
            continue;
        } else if (entry->d_type == DT_REG) {
            len = strlen(entry->d_name) + strlen(path);
            if (is_objfile_(entry->d_name, len - strlen(path))) {
                if (cmd_used + len + 1 >= cmd_len) {
                    cmd_len <<= 1;
                    cmd = realloc(cmd, cmd_len);
                }
                cmd_used += len + 2;
                strcat(cmd, " ");
                strcat(cmd, path);
                if (path[strlen(path) - 1] != '/') {
                    strcat(cmd, "/");
                }
                strcat(cmd, entry->d_name);
            }
        }
    }
    printf(GREEN "compiling: " RESET "%s\n", cmd);
    system(cmd);
}

static int run_command_(char* comp, ...)
{
    va_list ap;
    va_start(ap, comp);
    char* cmd = malloc(256);
    uint64_t len = 256;
    uint64_t new_len = strlen(comp);
    strcpy(cmd, comp);
    char* next_arg = NULL;

    while ((next_arg = va_arg(ap, char*)) != NULL) {
        new_len += strlen(next_arg) + 3;
        if (new_len >= len) {
            cmd = realloc(cmd, new_len << 1);
            len = new_len << 1;
        }
        strcat(cmd, " ");
        strcat(cmd, next_arg);
    }

    int r = system(cmd);
    free(cmd);
    return r;
}

#endif
