#ifndef SET_H
#define SET_H

#include "ds_error.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct set_t set_t;

typedef uint64_t (*set_hash_function_t)(void* key);
typedef bool (*set_cmp_function_t)(void* a, void* b);

typedef uintptr_t set_iterator_t;


bool set_contains(set_t* set, void* key);
void set_free(set_t* set);

ds_error_t set_create(set_t** set, set_hash_function_t hash, set_cmp_function_t cmp, uint64_t initial_size);
ds_error_t set_insert(set_t* set, void* key);

void set_delete(set_t* set, void* key);
uint64_t set_size(set_t* set);

void* set_iterator_value(set_t* set, set_iterator_t iter);
set_iterator_t set_iterator_next(set_t* set, set_iterator_t iter);
void* set_iterator_key(set_t* set, set_iterator_t iter);

#define SET_ITERATOR_INIT ((set_iterator_t)-1)
#define SET_ITERATOR_END ((set_iterator_t)-1)

// some hashfunctions ----------------------------------------------------------------

//char* hashfunctions null terminated
uint64_t set_simple_str_hash(void* key);
uint64_t set_str_djb2(void* key);
uint64_t set_str_jenkins(void* key);

//char* cmp function
bool set_str_cmp(void* a, void* b);

//int cmp functions
bool set_i64_cmp(void* a, void* b);
bool set_i32_cmp(void* a, void* b);
bool set_i16_cmp(void* a, void* b);
bool set_i8_cmp(void* a, void* b);

//uint cmp functions
bool set_u64_cmp(void* a, void* b);
bool set_u32_cmp(void* a, void* b);
bool set_u16_cmp(void* a, void* b);
bool set_u8_cmp(void* a, void* b);

//interprets block as uints and returns it probably best for hashing ints
uint64_t set_b64_self(void* key);
uint64_t set_b32_self(void* key);
uint64_t set_b16_self(void* key);
uint64_t set_b8_self(void* key);

#endif
