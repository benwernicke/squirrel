#include "config.h"
#include <ctype.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
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
    curl = curl_easy_init();
    if (curl != NULL) {
        fh = fopen(path, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fh);
        curl_easy_perform(curl);
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

void configure_includes_of_file(char* path)
{
    char* file_content = read_file_to_memory(path);
    char* s = file_content;
    char* prev = s;

    FILE* fh = fopen(path, "w");
    if (fh == NULL) {
        fprintf(stderr, RED "Error: " RESET "could not open: %s for include configuration: %s\n", path, strerror(errno));
        exit(1);
    }

    do {
        while (*s && strncmp(s, "#include", 8) != 0) {
            s++;
        }
        while (*s && *s != '"' && *s != '<') {
            s++;
        }
        if (*s == '"') {
            *s = '\0';
            fprintf(fh, "%s", prev);
            while (*s != '"') {
                s++;
            }
            *s = '\0';
            s--;
            while (*s && *s != '/') {
                s--;
            }
            s++;
            fprintf(fh, "\"%s\"", s);
            s++;
            while (*s) {
                s++;
            }
            s++;
            prev = s;
        }
    } while (*s);
    fprintf(fh, "%s", prev);

    fclose(fh);
    free(file_content);
}

void configure_includes(src_t* src_files, uint64_t len)
{
    uint64_t i;
    fprintf(stdout, GREEN "Configuring includes:\n" RESET);
    char name[132];
    if (len == 0) {
        fprintf(stdout, "\t--none--\n");
    }
    for (i = 0; i < len; i++) {
        strcpy(name, "lib/");
        strcat(name, src_files[i].file_name);
        fprintf(stdout, "\t%s\n", name);
        configure_includes_of_file(name);
    }
}

#define cat_path(b, l, ...) cat_path_(b, l, __VA_ARGS__, NULL)

int cat_path_(char** buf, uint64_t* buf_len, ...)
{
    va_list ap;
    va_start(ap, buf_len);

    char* next_arg = va_arg(ap, char*);
    uint64_t curr_len = strlen(next_arg);
    if (curr_len >= *buf_len) {
        *buf_len += (curr_len << 1);
        *buf = realloc(*buf, *buf_len);
        if (buf == NULL) {
            return -1;
        }
    }
    strcpy(*buf, next_arg);

    bool add_slash = 0;
    while ((next_arg = va_arg(ap, char*)) != NULL) {
        add_slash = 0;
        if ((*buf)[curr_len - 1] != '/') {
            add_slash = 1;
        }
        curr_len += strlen(next_arg) + add_slash;
        if (curr_len >= *buf_len) {
            *buf_len += (curr_len << 1);
            *buf = realloc(*buf, *buf_len);
            if (*buf == NULL) {
                return -1;
            }
        }
        if (add_slash) {
            strcat(*buf, "/");
        }
        strcat(*buf, next_arg);
    }
    va_end(ap);
    return 0;
}

void add_compile_instructions(src_t* src_files, uint64_t len)
{
    // check if compile instructions are needed
    {
        bool all_none_src = 1;
        uint64_t i;
        for (i = 0; i < len; i++) {
            if (src_files[i].type == SRC_FILE) {
                all_none_src = 0;
            }
        }
        if (all_none_src) { // len == 0 implicitly in there
            fprintf(stdout, GREEN "Adding compile instructions: " RESET "\n");
            fprintf(stdout, "\t--none--\n");
            return;
        }
    }
    // load file and init parsing pointer s
    char* file_content;
    char* s;
    {
        file_content = read_file_to_memory("cbuild.c");
        s = file_content;
    }

    FILE* fh = NULL;
    {
        fh = fopen("cbuild.c", "w");
        if (fh == NULL) {
            fprintf(stderr, RED "Error: " RESET "could not open cbuild.c: %s\n", strerror(errno));
            exit(1);
        }
    }
    // find pointer for assertion .. after auto_update[...]()[...];
    {
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
    }
    // dump previously parsed stuff until after auto_update(); insert '\0' for cstr and store char in tmp for later
    char tmp;
    {
        tmp = *s;
        *s = '\0';
        fprintf(fh, "%s\n", file_content);
    }
    // actually add compile instruction for every source file (.type == SRC_FILE) in src_files
    {
        fprintf(stdout, GREEN "Adding compile instructions: " RESET "\n");
        uint64_t i;
        fprintf(fh, "\n//Added by squirrel\n");
        char* path_buf = NULL;
        uint64_t path_buf_len = 0;
        for (i = 0; i < len; i++) {
            if (src_files[i].type == SRC_FILE) {
                if (cat_path(&path_buf, &path_buf_len, "lib/", src_files[i].file_name) != 0) {
                    fprintf(stderr, RED "Error: " RESET "could not cat path, probably bad_alloc\n");
                    exit(1);
                }
                fprintf(stdout, "\t%s\n", path_buf);
                fprintf(fh, "compile_object(\"%s\", \"%s\", \"build/", path_buf, src_files[i].compile_flags);
                path_buf[strlen(path_buf) - 1] = 'o';
                fprintf(fh, "%s\");\n", path_buf);
            }
        }
        free(path_buf);
        fprintf(fh, "//Not Added by squirrel\n");
    }
    // dump rest of file_content into cbuild.c
    {
        *s = tmp;
        fprintf(fh, "%s", s);
    }

    // cleanup
    {
        free(file_content);
        fclose(fh);
    }
}

bool file_exists(char* path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    } else {
        fclose(f);
        return 1;
    }
}

void stolower(char* s)
{
    while (*s) {
        if (isalpha(*s)) {
            *s = tolower(*s);
        }
        s++;
    }
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
    bool download_curr_file = 1;
    char* path_buf = NULL;
    uint64_t path_buf_len = 0;
    for (i = 0; i < len; i++) {
        if (strcmp(identifier, src[i].identifier) == 0) {
            download_curr_file = 1;
            if (cat_path(&path_buf, &path_buf_len, "lib/", src[i].file_name) != 0) {
                fprintf(stderr, RED "Error: " RESET "while cat_path, probably bad_alloc\n");
                exit(1);
            }
            if (file_exists(path_buf)) {
                char* s = NULL;
                int64_t len = 0;
                do {
                    fprintf(stdout, "\n\t%s already exists, still download? [y/n]: ", path_buf);
                    len = getline(&s, (uint64_t*)&len, stdin);
                    stolower(s);
                } while (strcmp(s, "y\n") != 0 && strcmp(s, "n\n") != 0);
                if (s[0] == 'n') {
                    download_curr_file = 0;
                }
                free(s);
            }
            if (download_curr_file) {
                fprintf(stdout, "\t%s from %s\n", path_buf, src[i].url);
                download_file(path_buf, src[i].url);
                if (src_files_used >= src_files_allocated) {
                    src_files_allocated <<= 1;
                    src_files = realloc(src_files, src_files_allocated * sizeof(*src_files));
                }
                src_files[src_files_used++] = src[i];
            }
        }
    }
    free(path_buf);

    add_compile_instructions(src_files, src_files_used);
    configure_includes(src_files, src_files_used);
    free(src_files);
}

void update(char* identifier)
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

            if (src_files_used >= src_files_allocated) {
                src_files_allocated <<= 1;
                src_files = realloc(src_files, src_files_allocated * sizeof(*src_files));
            }
            src_files[src_files_used++] = src[i];
        }
    }
    configure_includes(src_files, src_files_used);
    free(src_files);
}

void update_cbuild()
{
    fprintf(stdout, GREEN "Downloading:\n" RESET "\tlib/cbuild.h from https://raw.githubusercontent.com/benwernicke/cbuild/main/cbuild.h\n");
    download_file("lib/cbuild.h", "https://raw.githubusercontent.com/benwernicke/cbuild/main/cbuild.h");
    system(COMPILER " cbuild.c -o cbuild -Ofast -march=native");
}

void parse_argv(char** argv)
{
    argv++;
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
    } else if (strcmp(*argv, "update") == 0) {
        argv++;
        if (*argv == NULL) {
            fprintf(stderr, RED "Error: " RESET "update expects identifier argument\n");
            exit(1);
        }
        if (strcmp(*argv, "cbuild") == 0) {
            update_cbuild();
        } else {
            update(*argv);
        }
    }
}

int main(int argc, char** argv)
{
    parse_argv(argv);
    return 0;
}
