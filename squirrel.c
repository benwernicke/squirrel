#include "config.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

#define GREEN "\033[32m"
#define RESET "\033[39m"
#define RED "\033[31m"

void download_file(char* path, char* url)
{
    CURL* curl;
    FILE* fh;
    CURLcode res;
    curl = curl_easy_init();
    if (curl != NULL) {
        fh = fopen(path, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fh);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fh);
    } else {
        fprintf(stderr, RED "Error: " RESET "could not download: %s from url: %s\n", path, url);
    }
}

void help()
{
}

char default_cbuild[] = "#define CBUILD\n"
                        "#include \"lib/cbuild.h\"\n"
                        "\n\n"
                        "int main(int argc, char** argv)\n"
                        "{\n"
                        "\tauto_update();\n"
                        "\treturn 0;\n"
                        "}\n";

void init()
{
    fprintf(stdout, GREEN "Creating directories: \n" RESET);
    fprintf(stdout, "\tlib/\n");
    mkdir("lib", S_IRWXU);
    fprintf(stdout, "\tbuild/\n");
    mkdir("build", S_IRWXU);

    fprintf(stdout, GREEN "Downloading:\n" RESET "\tlib/cbuild.h from https://raw.githubusercontent.com/benwernicke/cbuild/main/cbuild.h\n");
    download_file("lib/cbuild.h", "https://raw.githubusercontent.com/benwernicke/cbuild/main/cbuild.h");

    FILE* fh_cbuild = fopen("cbuild.c", "w");
    if (fh_cbuild != NULL) {
        fprintf(stdout, GREEN "Creating:\n " RESET "\tdefault cbuild.c\n");
        fprintf(fh_cbuild, "%s", default_cbuild);
        fclose(fh_cbuild);
    } else {
        fprintf(stderr, RED "Error: " RESET "could not open cbuild.c: %s\n", strerror(errno));
    }
    fprintf(stdout, GREEN "Compiling: \n" RESET "\tcbuild.c\n");
    system(COMPILER " cbuild.c -o cbuild -Ofast -march=native");
}

char* read_file_to_memory(char* file_name)
{
    FILE* fh = fopen(file_name, "rb");
    if (fh == NULL) {
        fprintf(stderr, RED "Error: " RESET "could not open %s: %s\n", file_name, strerror(errno));
        exit(1);
    }
    fseek(fh, 0, SEEK_END);
    uint64_t fh_len = ftell(fh);
    rewind(fh);
    char* file_content = calloc(1, fh_len + 3);
    fread(file_content, 1, fh_len, fh);
    fclose(fh);
    return file_content;
}

void add_compile_instructions(src_t* src_files, uint64_t len)
{
    char* file_content = read_file_to_memory("cbuild.c");
    char* s = file_content;

    FILE* fh = fopen("cbuild.c", "w");
    if (fh == NULL) {
        fprintf(stderr, RED "Error: " RESET "could not open cbuild.c: %s\n", strerror(errno));
        exit(1);
    }

    while (*s && strncmp(s, "auto_update", strlen("auto_update")) != 0) {
        s++;
    }
    if (*s == '\0') {
        fprintf(stderr, RED "Error: " RESET "could not add compile instructions: cbuild.c does not contain auto_update(); consider adding it :)\n");
        exit(1);
    }
    while (*s && *s != ';') {
        s++;
    }
    if (*s == '\0') {
        fprintf(stderr, RED "Error: " RESET "syntax error in cbuild.c\n");
        exit(1);
    }
    s++;
    char tmp = *s;
    *s = '\0';
    fprintf(fh, "%s\n", file_content);

    uint64_t i;
    fprintf(fh, "\n//Added by squirrel\n");
    for (i = 0; i < len; i++) {
        char name[7 + strlen(src_files[i].file_name)];
        strcpy(name, "lib/");
        strcat(name, src_files[i].file_name);
        fprintf(fh, "compile_object(\"%s\", \"%s\", \"build/", name, src_files[i].compile_flags);
        strcpy(name, src_files[i].file_name);
        name[strlen(name) - 1] = 'o';
        fprintf(fh, "%s\");\n", name);
    }
    fprintf(fh, "//Not Added by squirrel\n");
    *s = tmp;
    fprintf(fh, "%s", s);
    free(file_content);
    fclose(fh);
}

void get(char* identifier)
{
    uint64_t len = 0;
    src_t* src = get_src(&len);
    uint64_t i;

    src_t* src_files = malloc(4 * sizeof(*src_files));
    uint64_t src_files_used = 0;
    uint64_t src_files_allocated = 4;

    fprintf(stdout, GREEN "Downloading:\n" RESET);
    for (i = 0; i < len; i++) {
        if (strcmp(identifier, src[i].identifier) == 0) {
            char name[5 + strlen(src[i].file_name)];
            strcpy(name, "lib/");
            strcat(name, src[i].file_name);
            fprintf(stdout, "\t%s from %s\n", name, src[i].url);
            download_file(name, src[i].url);

            if (src[i].type == SRC_FILE) {
                if (src_files_used >= src_files_allocated) {
                    src_files_allocated <<= 1;
                    src_files = realloc(src_files, src_files_allocated * sizeof(*src_files));
                }
                src_files[src_files_used++] = src[i];
            }
        }
    }

    add_compile_instructions(src_files, src_files_used);
    free(src_files);
}

void parse_argv(char** argv)
{
    while (*argv) {
        if (strcmp(*argv, "init") == 0) {
            init();
            return;
        } else if (strcmp(*argv, "help") == 0 || strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0) {
            help();
            return;
        } else if (strcmp(*argv, "get") == 0) {
            argv++;
            if (*argv == NULL) {
                fprintf(stderr, RED "Error: " RESET "get expects identifier argument\n");
                exit(1);
            }
            get(*argv);
        }
        argv++;
    }
}

int main(int argc, char** argv)
{
    parse_argv(argv);
    return 0;
}
