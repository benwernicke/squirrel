#include "set.h"

static inline int check_realloc_(set_t* set);
static inline SET_INT set_position_(set_t* set, SET_INT hash, void* key, SET_INT* buf_position, SET_INT* chain);

static inline SET_INT stack_pop_(set_t* set, SET_INT* stack);
static inline void stack_push_buf_(set_t* set, SET_INT* stack, SET_INT iter, SET_INT size);
static inline void stack_push_(set_t* set, SET_INT* stack, SET_INT i);
static inline SET_INT set_size_(set_t* set);

typedef struct entry_t entry_t;
struct entry_t {
    SET_INT hash;
    void* key;
    SET_INT next;
    SET_INT prev;
    SET_INT chain;
};

struct set_t {
    SET_INT buf_allocated;
    entry_t* buf;
    SET_INT* set;

    SET_INT unused_stack;
    SET_INT used_stack;

    set_hash_function_t hash;
    set_cmp_function_t cmp;

    SET_INT size;
};

static inline SET_INT set_size_(set_t* set)
{
    return set->buf_allocated << 2;
}

set_t* set_create(set_cmp_function_t cmp, SET_INT initial_size)
{
    set_t* set = malloc(sizeof(*set));
    if (set == NULL) {
        return NULL;
    }
    set->buf_allocated = initial_size;
    set->unused_stack = -1;
    set->used_stack = -1;
    set->size = 0;
    set->cmp = cmp;

    set->buf = malloc(initial_size * sizeof(*set->buf));
    set->set = malloc(set_size_(set) * sizeof(*set->set));
    memset(set->set, 0xFF, set_size_(set) * sizeof(*set->set));
    if (set->buf == NULL || set->set == NULL) {
        free(set->buf);
        free(set->set);
        free(set);
        return NULL;
    }
    stack_push_buf_(set, &set->unused_stack, 0, set->buf_allocated);
    return set;
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

static inline void stack_push_(set_t* set, SET_INT* stack, SET_INT i)
{
    set->buf[i].prev = -1;
    if (*stack != -1) {
        set->buf[*stack].prev = i;
    }
    set->buf[i].next = *stack;
    *stack = i;
}

static inline void stack_push_buf_(set_t* set, SET_INT* stack, SET_INT iter, SET_INT size)
{
    for (; iter < size; iter++) {
        stack_push_(set, stack, iter);
    }
}

static inline SET_INT stack_pop_(set_t* set, SET_INT* stack)
{
    SET_INT r = *stack;
    *stack = set->buf[*stack].next;
    if (*stack != -1) {
        set->buf[*stack].prev = -1;
    }
    return r;
}

static inline SET_INT stack_pop_position_(set_t* set, SET_INT* stack, SET_INT pos)
{
    SET_INT prev = set->buf[pos].prev;
    SET_INT next = set->buf[pos].next;
    if (next != -1) {
        set->buf[next].prev = prev;
    }
    if (prev != -1) {
        set->buf[prev].next = next;
    }
    return pos;
}
static inline SET_INT set_position_(set_t* set, SET_INT hash, void* key, SET_INT* buf_position, SET_INT* chain)
{
    hash %= set_size_(set);
    *buf_position = set->set[hash];
    *chain = -1;
    while (*buf_position != -1 && !set->cmp(key, set->buf[*buf_position].key)) {
        *chain = *buf_position;
        *buf_position = set->buf[*buf_position].chain;
    }
    return hash;
}

static inline int check_realloc_(set_t* set)
{
    if (set->unused_stack != -1) {
        return 0;
    }

    SET_INT old_allocated = set->buf_allocated;
    set->buf_allocated <<= 1;
    entry_t* new_buf = realloc(set->buf, set->buf_allocated * sizeof(*new_buf));
    SET_INT* new_set = realloc(set->set, set_size_(set) * sizeof(*new_set));

    if (new_set == NULL || new_buf == NULL) {
        set->buf_allocated = old_allocated;
        free(new_buf);
        free(new_set);
        return -1;
    }

    set->buf = new_buf;
    set->set = new_set;
    memset(set->set, 0xFF, set_size_(set) * sizeof(*set->set));
    stack_push_buf_(set, &set->unused_stack, old_allocated, set->buf_allocated);

    SET_INT i;
    SET_INT chain;
    SET_INT buf_position;
    SET_INT hash;
    for (i = 0; i < old_allocated; i++) {
        hash = set_position_(set, set->buf[i].hash, set->buf[i].key, &buf_position, &chain);
        if (chain == -1) {
            set->set[hash] = i;
        } else {
            set->buf[chain].chain = i;
        }
        set->buf[i].chain = -1;
    }
    return 1;
}

int set_insert(set_t* set, SET_INT hash, void* key)
{
    {
        int err = check_realloc_(set);
        if (err < 0) {
            return err;
        }
    }

    SET_INT buf_pos;
    SET_INT chain;
    SET_INT set_pos = set_position_(set, hash, key, &buf_pos, &chain);

    if (buf_pos == -1) {
        buf_pos = stack_pop_(set, &set->unused_stack);
        stack_push_(set, &set->used_stack, buf_pos);
        set->buf[buf_pos].key = key;
        set->buf[buf_pos].hash = hash;
        set->size++;
        set->buf[buf_pos].chain = -1;
        if (chain == -1) {
            set->set[set_pos] = buf_pos;
        } else {
            set->buf[chain].chain = buf_pos;
        }
    }
    return 0;
}

void set_delete(set_t* set, SET_INT hash, void* key)
{
    SET_INT chain;
    SET_INT buf_position;
    SET_INT set_pos = set_position_(set, hash, key, &buf_position, &chain);
    if (buf_position != -1) {
        stack_pop_position_(set, &set->used_stack, buf_position);
        stack_push_(set, &set->unused_stack, buf_position);
        set->size--;
        if (chain == -1) {
            set->set[set_pos] = -1;
        } else {
            set->buf[chain].chain = -1;
        }
    }
}

void* set_iterator_key(set_t* set, set_iterator_t iter)
{
    return set->buf[iter].key;
}

set_iterator_t set_iterator_next(set_t* set, set_iterator_t iter)
{
    if (iter == (set_iterator_t)-1) {
        return set->used_stack;
    }
    return set->buf[iter].next;
}

SET_INT set_size(set_t* set)
{
    return set->size;
}

bool set_contains(set_t* set, SET_INT hash, void* key)
{
    SET_INT buf_position;
    SET_INT chain;
    set_position_(set, hash, key, &buf_position, &chain);
    return buf_position != -1;
}
