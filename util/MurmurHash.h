#pragma once

#ifdef WINDOWS
#include <base/Log.h>
#else
#include <glog/logging.h>
#endif

class MurmurHash {
    public:
        #ifdef DEBUG
        static inline unsigned staticHash(const char *s, unsigned value) {
            LOG_IF(FATAL, compute(s, strlen(s), 0) != value) << "Invalid precalculated hash value. Expected: " << compute(s, strlen(s), 0)  << ". Actual: " << value;
            return value;
        }
        #else
        #define staticHash(s,v) (v)
        #endif

        static unsigned int compute( const void * key, int len, unsigned int seed );
};
