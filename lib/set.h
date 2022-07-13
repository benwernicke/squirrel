#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef SET_INT
#define SET_INT uint64_t
#endif

typedef struct set_t set_t;

typedef SET_INT (*set_hash_function_t)(void* key);
typedef bool (*set_cmp_function_t)(void* a, void* b);

typedef SET_INT set_iterator_t;


set_t* set_create(set_cmp_function_t cmp, SET_INT initial_size);
void set_free(set_t* set);

int set_insert(set_t* set, SET_INT hash, void* key);
void set_delete(set_t* set, SET_INT hash, void* key);
bool set_contains(set_t* set, SET_INT hash, void* key);

SET_INT set_size(set_t* set);

void* set_iterator_value(set_t* set, set_iterator_t iter);
set_iterator_t set_iterator_next(set_t* set, set_iterator_t iter);
void* set_iterator_key(set_t* set, set_iterator_t iter);

#define SET_ITERATOR_INIT ((set_iterator_t)-1)
#define SET_ITERATOR_END ((set_iterator_t)-1)

#endif
