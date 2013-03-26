#pragma once

#include "base/Log.h"
#include <cstring>

class MurmurHash {
    public:
        #ifdef SAC_DEBUG
        static inline unsigned staticHash(const char *s, unsigned value) {
            LOGF_IF(compute(s, strlen(s), 0) != value, "Invalid precalculated hash value. Expected: " << compute(s, strlen(s), 0)  << ". Actual: " << value)
            return value;
        }
        #else
        #define staticHash(s,v) (v)
        #endif

        static unsigned int compute( const void * key, int len, unsigned int seed );
};
