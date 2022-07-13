#include "hash.h"

HASH_INT map_str_jenkins(void* key)
{
    char* s = key;
    HASH_INT hash = 0;
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

HASH_INT hash_str_djb2(void* key)
{
    char* s = key;
    char c;
    HASH_INT hash = 5381;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

HASH_INT hash_str_ndjb2(void* key, HASH_INT len)
{
    HASH_INT hash = 5381;
    while (len > 4) {
        hash = ((hash << 5) + hash) + *(uint32_t*)key;
        len -= 4;
        key = (char*)key + 4;
    }
    while (len > 0) {
        hash = ((hash << 5) + hash) + *(char*)key;
        len -= 1;
        key = (char*)key + 1;
    }
    return hash;
}

HASH_INT hash_str_njenkins(void* key, HASH_INT len)
{
    HASH_INT hash = 0;
    while (len > 4) {
        hash += *(uint32_t*)key;
        hash += hash << 10;
        hash ^= hash >> 6;

        len -= 4;
        key = (char*)key + 4;
    }
    while (len > 0) {
        hash += *(uint8_t*)key;
        hash += hash << 10;
        hash ^= hash >> 6;

        len -= 1;
        key = (char*)key + 1;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

HASH_INT hash_nxor(void* key, HASH_INT len)
{

    HASH_INT hash = (HASH_INT)-1;
    while (len > 4) {
        hash ^= *(uint32_t*)key;

        len -= 4;
        key = (char*)key + 4;
    }
    while (len > 0) {
        hash ^= *(uint8_t*)key;

        len -= 1;
        key = (char*)key + 1;
    }
    return hash;
}
