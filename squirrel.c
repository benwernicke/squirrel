#include "config.h"
#include "lib/cmp.h"
#include "lib/hash.h"
#include "lib/panic.h"
#include "lib/path.h"
#include "lib/set.h"
#include "lib/vector.h"
#include "type.h"

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
//.................----------------------------------------------------------------
char help_page[] = "sqirrel\n"
                   "~~~~~~~\n"
                   "squirrel usage               print this page and exit\n"
                   "\n"
                   "squirrel init                will create default file system, download\n"
                   "                             cbuild.h, create cbuild.c and compile it\n"
                   "\n"
                   "sqirrel get <identifier>     download files associated with <identifier>\n"
                   "                             configure the include statements from those\n"
                   "                             files and add compile instructions to 'cbuild.c'\n"
                   "                             if AUTO_COMPILE_INSTRUCTIONS is defined\n"
                   "\n"
                   "squirrel update <identifier> download files associated with <identifier>\n"
                   "                             and configure the include statements from those\n"
                   "\n"
                   "\n";

void help()
{
    puts(help_page);
}

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
    FILE* fh_main = fopen("main.c", "w");
    panic_if(fh_cbuild == NULL || fh_main == NULL, "could not create 'cbuild.c' and 'main.'");
    fprintf(stdout, GREEN "Creating: \n" RESET);
    fprintf(stdout, "\tcbuild.c\n");
    fprintf(stdout, "\tmain.c\n");
    fprintf(fh_cbuild, "%s", get_default_cbuild());
    fprintf(fh_main, "%s", get_default_main());
    fclose(fh_cbuild);
    fclose(fh_main);

    fprintf(stdout, GREEN "Compiling: \n" RESET "\tcbuild.c\n");
    system(COMPILER " cbuild.c -o cbuild");
}

char* read_file_to_memory(char* file_name)
{
    FILE* fh = fopen(file_name, "rb");
    panic_if(fh == NULL, "could not open %s: %s", file_name, strerror(errno));
    fseek(fh, 0, SEEK_END);
    uint64_t fh_len = ftell(fh);
    rewind(fh);
    char* file_content = calloc(1, fh_len + 3);
    fread(file_content, 1, fh_len, fh);
    fclose(fh);
    return file_content;
}

bool is_include_statement(void* s)
{
    return strncmp(s, "#include", 8) == 0;
}

void configure_includes_of_file(char* path)
{
    char* file_content = read_file_to_memory(path);
    char* s = file_content;
    char* prev = s;

    FILE* fh = fopen(path, "w");
    panic_if(fh == NULL, "could not open: %s for include configuration: %s", path, strerror(errno));

    s = strstr(s, "#include");
    while (s != NULL) {
        s = strpbrk(s, ";<");
        if (*s == '"') {
            *s = '\0';
            fprintf(fh, "%s", prev);
            prev = ++s;
            s = index(s, '"');
            *s = '\0';
            s = rindex(prev, '/');
            s++;
            fprintf(fh, "\"%s\"", s);
            s = index(s, '\0');
            s++;
            prev = s;
        }
        s = strstr(s, "#include");
    }
    fprintf(fh, "%s", prev);

    fclose(fh);
    free(file_content);
}

void configure_includes(vector_t* files_to_configure)
{
    fprintf(stdout, GREEN "Configuring includes:\n" RESET);
    uint64_t size = vector_size(files_to_configure);
    if (size == 0) {
        fprintf(stdout, "\t--none--\n");
    }
    char* path_buf = NULL;
    uint64_t path_buf_len = 0;
    src_t** iter = (src_t**)vector_begin(files_to_configure);
    src_t** end = (src_t**)vector_end(files_to_configure);
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
        src_t** iter = (src_t**)vector_begin(files_to_add);
        src_t** end = (src_t**)vector_end(files_to_add);
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
        panic_if(fh == NULL, "could not open cbuild.c: %s", strerror(errno));
    }

    // find pointer for assertion .. after auto_update[...]()[...];
    {
        s = strstr(s, "auto_update");
        panic_if(s == NULL, "could not add, compile instructions: cbuild.c does not contain 'auto_update();' consider adding it :)");
        s = index(s, ';');
        panic_if(*s == '\0', "syntax error in cbuild.c");
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
        src_t** iter = (src_t**)vector_begin(files_to_add);
        src_t** end = (src_t**)vector_end(files_to_add);
        fprintf(fh, "\n//Added by squirrel\n");
        char* path_buf = NULL;
        uint64_t path_buf_len = 0;

        for (; iter != end; iter++) {
            if ((*iter)->type == SRC_FILE) {
                path_cat_or_panic(&path_buf, &path_buf_len, "build/", (*iter)->file_name);
                path_buf[strlen(path_buf) - 1] = 'o';
                fprintf(fh, "compile_object(\"%s\", FLAGS, \"%s\", ", path_buf, (*iter)->compile_flags);
                path_cat_or_panic(&path_buf, &path_buf_len, "lib/", (*iter)->file_name);
                fprintf(fh, "\"%s\");\n", path_buf);
                fprintf(stdout, "\t%s\n", path_buf);
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
    vector_t* files = vector_create(4);
    panic_if(files == NULL, "could not allocate files buffer: bad_alloc");
    uint64_t src_len = 0;

    // iterate over src from config.c and push to vec if identifier match
    {
        src_t* src = get_src(&src_len);
        src_t** files_more = NULL;
        uint64_t i;
        for (i = 0; i < src_len; i++) {
            if (strcmp(identifier, src[i].identifier) == 0) {
                files_more = (src_t**)vector_more(files);
                panic_if(files_more == NULL, "could not realloc files buffer: bad_alloc");
                *files_more = &src[i];
            }
        }
    }

    // check if anything was found if not panic with error msg -> used probably typed identifier wrong
    {
        uint64_t size = vector_size(files);
        panic_if(size == 0, "could not find any src declarations with identifier: %s", identifier);
    }

    return files;
}

void download_files(vector_t* files_to_download)
{
    src_t** iter = (src_t**)vector_begin(files_to_download);
    src_t** end = (src_t**)vector_end(files_to_download);
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
    fprintf(stdout, GREEN "Compiling:\n" RESET);
    fprintf(stdout, "\t" COMPILER " cbuild.c -o cbuild -Ofast -march=native\n");
    system(COMPILER " cbuild.c -o cbuild -Ofast -march=native");
}

void dump_identifier()
{
    uint64_t src_len = 0;
    src_t* src = get_src(&src_len);

    set_t* seen = set_create(cmp_str, src_len);
    panic_if(seen == NULL, "could not create seen hashset: bad_alloc");

    uint64_t i;
    uint64_t hash;
    printf("Availabe identifier: \n");
    for (i = 0; i < src_len; i++) {
        hash = hash_str_djb2(src[i].identifier);
        if (!set_contains(seen, hash, src[i].identifier)) {
            printf("\t%s\n", src[i].identifier);
            set_insert(seen, hash, src[i].identifier);
        }
    }
    set_free(seen);
}

void info(char* identifier)
{
    vector_t* src = find_src_by_identifier(identifier);

    src_t** iter = (src_t**)vector_begin(src);
    src_t** end = (src_t**)vector_end(src);

    fprintf(stdout, GREEN "Info: " RESET "%s\n", identifier);
    for (; iter != end; iter++) {
        printf("\t%s:\n", (*iter)->file_name);
        printf("\t\tfrom:\t%s\n", (*iter)->url);
        printf("\t\ttype:\t");
        switch ((*iter)->type) {
        case SRC_FILE:
            printf("source file\n");
            printf("\t\tflags:\t%s\n", (*iter)->compile_flags);
            break;
        case SRC_HEADER:
            printf("header file\n");
            break;
        case SRC_OTHER:
            printf("neither source nor header file\n");
            break;
        }
        printf("\n");
    }

    vector_free(src);
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
        panic_if(*argv == NULL, "get expects identifier argument");
        get(*argv);
    } else if (strcmp(*argv, "update") == 0) {
        argv++;
        panic_if(*argv == NULL, "update expects identifier argument");
        if (strcmp(*argv, "cbuild") == 0) {
            update_cbuild();
        } else {
            update(*argv);
        }
    } else if (strcmp(*argv, "usage") == 0) {
        help();
    } else if (strcmp(*argv, "identifiers") == 0) {
        dump_identifier();
    } else if (strcmp(*argv, "info") == 0) {
        argv++;
        panic_if(*argv == NULL, "info expects identifier argument");
        info(*argv);
    } else {
        panic("mode: '%s' was not found see 'sqirrel usage' for usage", *argv);
    }
}

int main(int argc, char** argv)
{
    putc('\n', stdout);
    parse_argv(argv);
    putc('\n', stdout);
    return 0;
}
