/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "MurmurHash.h"


#if 0
#define M 0x5bd1e995
#define R 24

static constexpr unsigned int mulM(unsigned int k) {
    return k * M;
}

static constexpr unsigned int selfExpShift(unsigned int h, int n) {
    return h ^ (h >> n);
}

static constexpr unsigned int loopInt(unsigned int k, unsigned int h) {
    return mulM(h) ^ mulM(selfExpShift(mulM(k), R));
}

static constexpr unsigned int leftOver(const unsigned char* data, unsigned int h, int len) {

    return (len == 0) ?
        h * M :
        leftOver(data, h ^ data[len - 1] << (8 * (len - 1)), len - 1);
}

static constexpr unsigned int loop(const unsigned char* key, int len, unsigned int h) {

    return (len >= 4) ?
        loop(key + 4, len - 4, loopInt(*(const unsigned int *)key, h)) :
        ((len > 0) ? leftOver(key, h, len) : h);
}

constexpr unsigned int MurmurHash::compute(const void * key, int len, unsigned int seed) {
    return
        selfExpShift(
            mulM(
                selfExpShift(
                    loop((const unsigned char*) key, len, seed ^ len)
                    , 13)
            )
        , 15);
}
#endif

// MurmurHash2, by Austin Appleby
uint32_t Murmur::RuntimeHash(const void * key, int len) {
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const unsigned int m = M;
    const int r = R;

    // Initialize the hash to a 'random' value
    unsigned int h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    const unsigned char * data = (const unsigned char *)key;

    while(len >= 4) {
        unsigned int k = *(unsigned int *)data;


        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array
    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];

        h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
