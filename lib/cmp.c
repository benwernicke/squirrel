#include "cmp.h"

bool cmp_str(void* a, void* b)
{
    return strcmp((char*)a, (char*)b) == 0;
}

bool cmp_u64(void* a, void* b)
{
    return *(uint64_t*)a == *(uint64_t*)b;
}

bool cmp_u32(void* a, void* b)
{
    return *(uint32_t*)a == *(uint32_t*)b;
}

bool cmp_u16(void* a, void* b)
{
    return *(uint16_t*)a == *(uint16_t*)b;
}

bool cmp_u8(void* a, void* b)
{
    return *(uint8_t*)a == *(uint8_t*)b;
}

bool cmp_i64(void* a, void* b)
{
    return *(int64_t*)a == *(int64_t*)b;
}

bool cmp_i32(void* a, void* b)
{
    return *(int32_t*)a == *(int32_t*)b;
}

bool cmp_i16(void* a, void* b)
{
    return *(int16_t*)a == *(int16_t*)b;
}

bool cmp_i8(void* a, void* b)
{
    return *(int8_t*)a == *(int8_t*)b;
}
