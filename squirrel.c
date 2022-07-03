#include "config.h"
/*#include "lib/buffer.h"*/
#include "lib/path.h"
#include "lib/vector.h"
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

vector_t* find_src_by_identifier(char* identifier);
void download_files(vector_t* files_to_download);

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

void configure_includes(vector_t* files_to_configure)
{
    fprintf(stdout, GREEN "Configuring includes:\n" RESET);
    uint64_t size;
    vector_size(files_to_configure, &size);
    if (size == 0) {
        fprintf(stdout, "\t--none--\n");
    }
    char* path_buf = NULL;
    uint64_t path_buf_len = 0;
    src_t** iter;
    src_t** end;
    vector_begin(files_to_configure, (void***)&iter);
    vector_end(files_to_configure, (void***)&end);
    for (; iter != end; iter++) {
        path_cat_or_panic(&path_buf, &path_buf_len, "lib/", (*iter)->file_name);
        fprintf(stdout, "\t%s\n", path_buf);
        configure_includes_of_file(path_buf);
    }
}

void add_compile_instructions(vector_t* files_to_add)
{
    // check if compile instructions are needed
    {
        src_t** iter;
        src_t** end;
        vector_begin(files_to_add, (void***)&iter);
        vector_end(files_to_add, (void***)&end);
        bool all_none_src = 1;

        for (; iter != end; iter++) {
            if ((*iter)->type == SRC_FILE) {
                all_none_src = 0;
                break;
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
        src_t** iter;
        src_t** end;
        vector_begin(files_to_add, (void***)&iter);
        vector_end(files_to_add, (void***)&end);
        fprintf(fh, "\n//Added by squirrel\n");
        char* path_buf = NULL;
        uint64_t path_buf_len = 0;

        for (; iter != end; iter++) {
            if ((*iter)->type == SRC_FILE) {
                path_cat_or_panic(&path_buf, &path_buf_len, "lib/", (*iter)->file_name);
                fprintf(stdout, "\t%s\n", path_buf);
                fprintf(fh, "compile_object(\"%s\", \"%s\", \"", path_buf, (*iter)->compile_flags);
                path_cat_or_panic(&path_buf, &path_buf_len, "build/", (*iter)->file_name);
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
    vector_t* files_to_get = find_src_by_identifier(identifier);
    download_files(files_to_get);
    configure_includes(files_to_get);

#ifdef AUTO_COMPILE_INSTRUCTIONS
    add_compile_instructions(files_to_get);
#endif

    vector_free(files_to_get);
}

vector_t* find_src_by_identifier(char* identifier)
{
    vector_t* files = NULL;
    if (vector_create(&files, 4) != SUCCESS) {
        fprintf(stderr, RED "Error: " RESET "could not allocate files buffer: bad_alloc\n");
        exit(1);
    }
    uint64_t src_len = 0;

    // iterate over src from config.c and push to vec if identifier match
    {
        src_t* src = get_src(&src_len);
        src_t** files_more = NULL;
        uint64_t i;
        for (i = 0; i < src_len; i++) {
            if (strcmp(identifier, src[i].identifier) == 0) {
                if (vector_more(files, (void***)&files_more) != SUCCESS) {
                    fprintf(stderr, RED "Error: " RESET "could not allocate files buffer: bad_alloc\n");
                    exit(1);
                }
                *files_more = &src[i];
            }
        }
    }

    // check if anything was found if not panic with error msg -> used probably typed identifier wrong
    {
        uint64_t size = 0;
        vector_size(files, &size);
        if (size == 0) {
            fprintf(stderr, RED "Error: " RESET "could not find any src declarations with identifier: %s\n", identifier);
            exit(1);
        }
    }

    return files;
}

void download_files(vector_t* files_to_download)
{
    src_t** iter;
    src_t** end;
    vector_begin(files_to_download, (void***)&iter);
    vector_end(files_to_download, (void***)&end);
    char* path_buf = NULL;
    uint64_t path_buf_len = 0;
    fprintf(stdout, GREEN "Downloading:\n" RESET);
    while (iter != end) {
        path_cat_or_panic(&path_buf, &path_buf_len, "lib/", (*iter)->file_name);
        fprintf(stdout, "\t%s from %s\n", path_buf, (*iter)->url);
        download_file(path_buf, (*iter)->url);
        iter++;
    }
    free(path_buf);
}

void update(char* identifier)
{
    vector_t* files_to_update = find_src_by_identifier(identifier);
    download_files(files_to_update);
    configure_includes(files_to_update);
    vector_free(files_to_update);
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
    } else if (strcmp(*argv, "usage") == 0) {
        help();
    } else {
        fprintf(stderr, RED "Error: " RESET "mode: '%s' was not found see 'sqirrel usage' for usage\n", *argv);
    }
}

int main(int argc, char** argv)
{
    parse_argv(argv);
    return 0;
}
