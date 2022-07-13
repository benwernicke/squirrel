#include "vector.h"

struct vector_t {
    uint64_t allocated;
    uint64_t used;
    void** buf;
};

vector_t* vector_create(uint64_t init_size)
{
    vector_t* v = malloc(sizeof(*v));
    if (v == NULL) {
        return NULL;
    }
    v->allocated = init_size;
    v->used = 0;
    v->buf = malloc(v->allocated * sizeof(*v->buf));
    if (v->buf == NULL) {
        free(v);
        return NULL;
    }
    return v;
}

void** vector_begin(vector_t* v)
{
    return v->buf;
}

void** vector_end(vector_t* v)
{
    return &v->buf[v->used];
}

uint64_t vector_size(vector_t* v)
{
    return v->used;
}

void** vector_more(vector_t* v)
{
    if (v->used >= v->allocated) {
        uint64_t new_allocated = v->allocated << 1;
        void** new_buf = realloc(v->buf, sizeof(*new_buf) * new_allocated);
        if (new_buf == NULL) {
            return NULL;
        }
        v->allocated = new_allocated;
        v->buf = new_buf;
    }
    return &v->buf[v->used++];
}

void* vector_get(vector_t* v, uint64_t index)
{
    return v->buf[index];
}

void vector_free(vector_t* v)
{
    if (v == NULL) {
        return;
    }
    free(v->buf);
    free(v);
}

void vector_remove(vector_t* v, uint64_t index)
{
    if (index == v->used - 1) {
        v->used--;
    }
    memmove(v->buf + index, v->buf + index + 1, (v->used - index) * sizeof(*v->buf));
    v->used--;
}
