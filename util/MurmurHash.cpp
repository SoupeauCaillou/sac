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


#if SAC_DEBUG || SAC_INGAME_EDITORS
std::map<uint32_t, const char*>* Murmur::_lookup = 0;

const char* Murmur::lookup(uint32_t t) {
    auto it = _lookup->find(t);
    if (it == _lookup->end()) {
        LOGE("Invalid lookup key: " << t);
        return "";
    } else {
        return it->second;
    }
}

uint32_t Murmur::verifyHash(const char* txt, uint32_t hash, const char* file, int line) {
    uint32_t h = RuntimeHash(txt, strlen(txt));
    LOGE_IF(h != hash, "Incorrect hash for '" << txt << "' at " << file << ':' << line << ". Expected: 0x" << std::hex << h << " and was 0x" << hash << std::dec);
    return h;
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

#if SAC_DEBUG || SAC_INGAME_EDITORS
    if (!_lookup) _lookup = new std::map<uint32_t, const char*>();
    (*_lookup)[h] = strdup((const char*)key);
#endif

    return h;
}
