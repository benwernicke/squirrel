#ifndef HASH_H
#define HASH_H

#ifndef HASH_INT
#define HASH_INT uint64_t
#endif

#include <stdint.h>

HASH_INT hash_nxor(void* key, HASH_INT len);
HASH_INT hash_str_njenkins(void* key, HASH_INT len);
HASH_INT hash_str_ndjb2(void* key, HASH_INT len);
HASH_INT hash_str_djb2(void* key);
HASH_INT map_str_jenkins(void* key);

#endif
