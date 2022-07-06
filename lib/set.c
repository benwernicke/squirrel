#include "set.h"

static ds_error_t check_realloc_(set_t* set);
static uint64_t set_position_(set_t* set, void* key);
static uint64_t stack_pop_(set_t* set, uint64_t* stack);
static void stack_push_buf_(set_t* set, uint64_t* stack, uint64_t iter, uint64_t size);
static void stack_push_(set_t* set, uint64_t* stack, uint64_t i);
static uint64_t set_size_(set_t* set);

typedef struct entry_t entry_t;
struct entry_t {
    void* key;

    uint64_t next;
    uint64_t prev;
};

struct set_t {
    uint64_t buf_allocated;
    entry_t* buf;
    uint64_t* set;

    uint64_t unused_stack;
    uint64_t used_stack;

    set_hash_function_t hash;
    set_cmp_function_t cmp;

    uint64_t size;
};

static uint64_t set_size_(set_t* set)
{
    return set->buf_allocated << 2;
}

ds_error_t set_create(set_t** set, set_hash_function_t hash, set_cmp_function_t cmp, uint64_t initial_size)
{
    *set = malloc(sizeof(**set));
    if (*set == NULL) {
        return BAD_ALLOC;
    }
    (*set)->buf_allocated = initial_size;
    (*set)->unused_stack = -1;
    (*set)->used_stack = -1;
    (*set)->size = 0;
    (*set)->hash = hash;
    (*set)->cmp = cmp;

    (*set)->buf = malloc(initial_size * sizeof(*(*set)->buf));
    (*set)->set= malloc(set_size_(*set) * sizeof(*(*set)->set));
    memset((*set)->set, 0xFF, set_size_(*set) * sizeof(*(*set)->set));
    if ((*set)->buf == NULL || (*set)->set == NULL) {
        free((*set)->buf);
        free((*set)->set);
        free(*set);
        return BAD_ALLOC;
    }
    stack_push_buf_(*set, &(*set)->unused_stack, 0, (*set)->buf_allocated);
    return SUCCESS;
}

void set_free(set_t* set)
{
    if (set == NULL) {
        return;
    }
    free(set->buf);
    free(set->set);
    free(set);
}

static void stack_push_(set_t* set, uint64_t* stack, uint64_t i)
{
    set->buf[i].prev = -1;
    if (*stack != -1) {
        set->buf[*stack].prev = i;
    }
    set->buf[i].next = *stack;
    *stack = i;
}

static void stack_push_buf_(set_t* set, uint64_t* stack, uint64_t iter, uint64_t size)
{
    for (; iter < size; iter++) {
        stack_push_(set, stack, iter);
    }
}

static uint64_t stack_pop_(set_t* set, uint64_t* stack)
{
    uint64_t r = *stack;
    *stack = set->buf[*stack].next;
    if (*stack != -1) {
        set->buf[*stack].prev = -1;
    }
    return r;
}

static uint64_t stack_pop_position_(set_t* set, uint64_t* stack, uint64_t pos)
{
    uint64_t prev = set->buf[pos].prev;
    uint64_t next = set->buf[pos].next;
    if (next != -1) {
        set->buf[next].prev = prev;
    }
    if (prev != -1) {
        set->buf[prev].next = next;
    }
    return pos;
}

static uint64_t set_position_(set_t* set, void* key)
{
    uint64_t i = set->hash(key) % set_size_(set);
    while (set->set[i] != -1 && !set->cmp(key, set->buf[set->set[i]].key)) {
        i = (i + 1) % set_size_(set);
    }
    return i;
}

static ds_error_t check_realloc_(set_t* set)
{
    if (set->unused_stack != -1) {
        return SUCCESS;
    }

    uint64_t old_allocated = set->buf_allocated;
    set->buf_allocated <<= 1;
    entry_t* new_buf = realloc(set->buf, set->buf_allocated * sizeof(*new_buf));
    uint64_t* new_set = realloc(set->set, set_size_(set) * sizeof(*new_set));

    if (new_set == NULL || new_buf == NULL) {
        set->buf_allocated = old_allocated;
        free(new_buf);
        free(new_set);
        return BAD_ALLOC;
    }

    set->buf = new_buf;
    set->set = new_set;
    memset(set->set, 0xFF, set_size_(set) * sizeof(*set->set));
    stack_push_buf_(set, &set->unused_stack, old_allocated, set->buf_allocated);

    uint64_t i;
    for (i = 0; i < old_allocated; i++) {
        set->set[set_position_(set, set->buf[i].key)] = i;
    }
    return SUCCESS;
}

ds_error_t set_insert(set_t* set, void* key)
{
    {
        ds_error_t err = check_realloc_(set);
        if (err != SUCCESS) {
            return err;
        }
    }
    uint64_t i = set_position_(set, key);
    if (set->set[i] == -1) {
        uint64_t n = stack_pop_(set, &set->unused_stack);
        stack_push_(set, &set->used_stack, n);
        if (n == -1) {
            return BAD_ALLOC;
        }
        set->set[i] = n;
        set->buf[set->set[i]].key = key;
        set->size++;
    }
    return SUCCESS;
}

void set_delete(set_t* set, void* key)
{
    uint64_t i = set_position_(set, key);
    uint64_t n = set->set[i];
    if (n != -1) {
        stack_pop_position_(set, &set->used_stack, n);
        stack_push_(set, &set->unused_stack, n);
        set->set[i] = -1;
        set->size--;
    }
}

void* set_iterator_key(set_t* set, set_iterator_t iter)
{
    return set->buf[iter].key;
}

set_iterator_t set_iterator_next(set_t* set, set_iterator_t iter)
{
    if (iter == -1) {
        return set->used_stack;
    }
    return set->buf[iter].next;
}

uint64_t set_size(set_t* set)
{
    return set->size;
}

//--------------------------------------------------------------------------------------------------------------------------------

uint64_t set_str_hash(void* key)
{
    char* s = key;
    uint64_t h = 86969;
    while (*s) {
        h = (h * 54059) ^ (s[0] * 76963);
        s++;
    }
    return h;
}

uint64_t set_str_jenkins(void* key)
{
    char* s = key;
    uint64_t hash = 0;
    char c;
    while ((c = *s++)) {
        hash += c;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

bool set_str_cmp(void* a, void* b)
{
    return strcmp((char*)a, (char*)b) == 0;
}

bool set_contains(set_t* set, void* key)
{
    uint64_t i = set_position_(set, key);
    return set->set[i] != -1;
}

uint64_t set_str_djb2(void* key)
{
    char* s = key;
    char c;
    uint64_t hash = 5381;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

bool set_i8_cmp(void* a, void* b)
{
    return *(int8_t*)a == *(int8_t*)b;
}

bool set_i16_cmp(void* a, void* b)
{
    return *(int16_t*)a == *(int16_t*)b;
}

bool set_i32_cmp(void* a, void* b)
{
    return *(int16_t*)a == *(int16_t*)b;
}

bool set_i64_cmp(void* a, void* b)
{
    return *(int64_t*)a == *(int64_t*)b;
}

bool set_u8_cmp(void* a, void* b)
{
    return *(uint8_t*)a == *(uint8_t*)b;
}

bool set_u16_cmp(void* a, void* b)
{
    return *(uint16_t*)a == *(uint16_t*)b;
}

bool set_u32_cmp(void* a, void* b)
{
    return *(uint16_t*)a == *(uint16_t*)b;
}

bool set_u64_cmp(void* a, void* b)
{
    return *(uint64_t*)a == *(uint64_t*)b;
}

uint64_t set_b64_self(void* key)
{
    return *(uint64_t*)key;
}

uint64_t set_b32_self(void* key)
{
    return *(uint32_t*)key;
}

uint64_t set_b16_self(void* key)
{
    return *(uint16_t*)key;
}
uint64_t set_b8_self(void* key)
{
    return *(uint8_t*)key;
}
