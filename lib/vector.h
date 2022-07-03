#ifndef VECTOR_H
#define VECTOR_H

#include "ds_error.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct vector_t vector_t;

ds_error_t vector_create(vector_t** v, uint64_t init_size);
ds_error_t vector_begin(vector_t* v, void*** begin);
ds_error_t vector_end(vector_t* v, void*** end);
ds_error_t vector_size(vector_t* v, uint64_t* size);
ds_error_t vector_more(vector_t* v, void*** more);
ds_error_t vector_get(vector_t* v, uint64_t index, void** ret);
ds_error_t vector_free(vector_t* v);
ds_error_t vector_remove(vector_t* v, uint64_t index);

#endif
