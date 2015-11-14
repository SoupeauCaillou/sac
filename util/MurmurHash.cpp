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
#include "base/Log.h"
#include <cstdlib>

#if SAC_DEBUG || SAC_INGAME_EDITORS
#include <map>
static std::map<uint32_t, char*>* _lookup = 0;

const char* Murmur::lookup(uint32_t t) {
    auto it = _lookup->find(t);
    if (it == _lookup->end()) {
        LOGE("Invalid lookup key: " << t);
        return "";
    } else {
        return it->second;
    }
}

uint32_t Murmur::verifyHash(const char* txt, uint32_t hash, const char* file, int) {
    uint32_t h = RuntimeHash(txt, strlen(txt));
    if (h != hash) {
        std::string escaped;
        for (unsigned i=0; i<strlen(txt); i++) {
            if (txt[i] == '/') escaped.push_back('\\');
            escaped.push_back(txt[i]);
        }
        LOGI_EVERY_N(5, "./RecursiveRunner | grep sed | cut -d: -f3 > fix_hashes then 'sh fix_hashes'");
        LOGE("sed 's/HASH(\"" << escaped << "\", 0x" << LogFmt("%08x") << hash << LogFmt() << ")/HASH(\"" << escaped << "\", 0x" << LogFmt("%08x") << h << LogFmt() << ")/' -i " << file);
    }
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

    const char * data = (const char *)key;

    while(len >= 4) {
        unsigned int k = readAsInt(data);
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
    if (!_lookup) {
        _lookup = new std::map<uint32_t, char*>();
    }
    if (_lookup->find(h) != _lookup->end()){
        free((*_lookup)[h]);
    }
    (*_lookup)[h] = strdup((const char*)key);
#endif

    return h;
}

void Murmur::destroy() {
#if SAC_DEBUG || SAC_INGAME_EDITORS
    if (_lookup) {
        for (auto & it : *_lookup) {
            free(it.second);
        }
        delete _lookup;
    }
#endif
}
