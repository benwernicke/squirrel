#ifndef CMP_H
#define CMP_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

bool cmp_str(void* a, void* b);
bool cmp_u64(void* a, void* b);
bool cmp_u32(void* a, void* b);
bool cmp_u16(void* a, void* b);
bool cmp_u8(void* a, void* b);
bool cmp_i64(void* a, void* b);
bool cmp_i32(void* a, void* b);
bool cmp_i16(void* a, void* b);
bool cmp_i8(void* a, void* b);

#endif
