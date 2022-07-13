#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct vector_t vector_t;

vector_t* vector_create(uint64_t init_size);
void** vector_begin(vector_t* v);
void** vector_end(vector_t* v);
uint64_t vector_size(vector_t* v);
void** vector_more(vector_t* v);
void* vector_get(vector_t* v, uint64_t index);
void vector_free(vector_t* v);
void vector_remove(vector_t* v, uint64_t index);

#endif
