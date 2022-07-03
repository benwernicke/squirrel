#include "buffer.h"

struct buffer_t {
    uint64_t allocated;
    uint64_t used;
    uint64_t member_size;
    uint8_t* buf;
};

ds_error_t buffer_create(buffer_t** b, uint64_t initial_size, uint64_t member_size)
{
    *b = malloc(sizeof(**b));
    if (*b == NULL) {
        return DS_BAD_ALLOC;
    }
    (*b)->allocated = initial_size;
    (*b)->used = 0;
    (*b)->member_size = member_size;
    (*b)->buf = malloc(initial_size * member_size);
    return DS_SUCCESS;
}

ds_error_t buffer_create_from_range(buffer_t** b, uint64_t initial_size, uint64_t member_size, void* range, uint64_t range_size)
{
    ds_error_t err = buffer_create(b, initial_size, member_size);
    if (err != DS_SUCCESS) {
        return err;
    }
    memcpy((*b)->buf, range, range_size);
    (*b)->used = range_size / member_size;
    return DS_SUCCESS;
}

ds_error_t buffer_get(buffer_t* b, uint64_t index, void** ret)
{
    if (index >= b->used) {
        *ret = NULL;
        return DS_INDEX_OUT_OF_BOUND;
    }
    *ret = b->buf + index * b->member_size;
    return DS_SUCCESS;
}

ds_error_t buffer_free(buffer_t* b)
{
    if (b == NULL) {
        return DS_SUCCESS;
    }
    free(b->buf);
    free(b);
    return DS_SUCCESS;
}

ds_error_t buffer_more(buffer_t* b, void** ret)
{
    if (b->used >= b->allocated) {
        uint64_t new_allocated = b->allocated << 1;
        uint8_t* new_buf = realloc(b->buf, new_allocated * b->member_size);
        if (new_buf == NULL) {
            *ret = NULL;
            return DS_BAD_ALLOC;
        }
        b->buf = new_buf;
        b->allocated = new_allocated;
    }
    *ret = b->buf + b->used++ * b->member_size;
    return DS_SUCCESS;
}

ds_error_t buffer_remove(buffer_t* b, uint64_t index)
{
    if (index >= b->used) {
        return DS_INDEX_OUT_OF_BOUND;
    }
    if (index == b->used - 1) {
        b->used--;
        return DS_SUCCESS;
    }
    memmove(b->buf + index * b->member_size, b->buf + (index + 1) * b->member_size, (b->used - index - 1) * b->member_size);
    b->used--;
    // TODO: realloc
    return DS_SUCCESS;
}

ds_error_t buffer_index_from_ptr(buffer_t* b, void* ptr, uint64_t* index)
{
    *index = (uint64_t)((uint8_t*)ptr - b->buf) / b->member_size;
    if (*index >= b->used) {
        return DS_INDEX_OUT_OF_BOUND;
    }
    return DS_SUCCESS;
}

ds_error_t buffer_remove_ptr(buffer_t* b, void* ptr)
{
    uint64_t index = 0;
    ds_error_t err = buffer_index_from_ptr(b, ptr, &index);
    if (err != DS_SUCCESS) {
        return err;
    }
    return buffer_remove(b, index);
}

ds_error_t buffer_begin(buffer_t* b, void** begin)
{
    *begin = b->buf;
    return DS_SUCCESS;
}

ds_error_t buffer_end(buffer_t* b, void** end)
{
    *end = b->buf + b->used;
    return DS_SUCCESS;
}

ds_error_t buffer_size(buffer_t* b, uint64_t* size)
{
    *size = b->used;
    return DS_SUCCESS;
}
