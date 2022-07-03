#include "vector.h"

struct vector_t {
    uint64_t allocated;
    uint64_t used;
    void** buf;
};

ds_error_t vector_create(vector_t** v, uint64_t init_size)
{
    *v = malloc(sizeof(**v));
    if (*v == NULL) {
        return BAD_ALLOC;
    }
    (*v)->allocated = init_size;
    (*v)->used = 0;
    (*v)->buf = malloc((*v)->allocated * sizeof(*(*v)->buf));
    if ((*v)->buf == NULL) {
        free(*v);
        return BAD_ALLOC;
    }
    return SUCCESS;
}

ds_error_t vector_begin(vector_t* v, void*** begin)
{
    *begin = v->buf;
    return SUCCESS;
}

ds_error_t vector_end(vector_t* v, void*** end)
{
    *end = &v->buf[v->used];
    return SUCCESS;
}

ds_error_t vector_size(vector_t* v, uint64_t* size)
{
    *size = v->used;
    return SUCCESS;
}

ds_error_t vector_more(vector_t* v, void*** more)
{
    if (v->used >= v->allocated) {
        uint64_t new_allocated = v->allocated << 1;
        void** new_buf = realloc(v->buf, sizeof(*new_buf) * new_allocated);
        if (new_buf == NULL) {
            *more = NULL;
            return BAD_ALLOC;
        }
        v->allocated = new_allocated;
        v->buf = new_buf;
    }
    *more = &v->buf[v->used++];
    return SUCCESS;
}

ds_error_t vector_remove(vector_t* v, uint64_t index)
{
    if (index >= v->used) {
        return INDEX_OUT_OF_BOUND;
    }
    if (index == v->used - 1) {
        v->used--;
        return SUCCESS;
    }
    memmove(v->buf + index, v->buf + index + 1, v->used * sizeof(*v->buf));
    v->used--;
    return SUCCESS;
}

ds_error_t vector_get(vector_t* v, uint64_t index, void** ret)
{
    if (index >= v->used) {
        return INDEX_OUT_OF_BOUND;
    }
    *ret = v->buf[index];
    return SUCCESS;
}

ds_error_t vector_free(vector_t* v)
{
    if (v == NULL) {
        return SUCCESS;
    }
    free(v->buf);
    free(v);
    return SUCCESS;
}
