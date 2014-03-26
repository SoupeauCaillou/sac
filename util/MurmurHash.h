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



#pragma once

#include "base/Log.h"
#include <cstring>

typedef uint32_t hash_t;

class Murmur {
    private:
        static constexpr uint32_t seed = 0x12345678;
    public:

        static hash_t RuntimeHash(const void * key, int len);

        static constexpr hash_t Hash(const void * key, int len) {
            return
                selfExpShift(
                    mulM(
                        selfExpShift(
                            loop((const char*) key, len, seed ^ len)
                            , 13)
                    )
                , 15);
        }

        // assume key contains \0
        static constexpr hash_t Hash(const char * key) {
            return Hash(key, strlen(key));
        }
    private:
        static constexpr uint32_t M = 0x5bd1e995;
        static constexpr int R = 24;

        static constexpr uint32_t mulM(uint32_t k) { return k * M; }

        static constexpr uint32_t selfExpShift(uint32_t h, int n) { return h ^ (h >> n); }

        static constexpr uint32_t loopInt(uint32_t k, uint32_t h) { return mulM(h) ^ mulM(selfExpShift(mulM(k), R)); }

        static constexpr uint32_t leftOver(const char* data, uint32_t h, int len) {
            return (len == 0) ?
                h * M :
                leftOver(data, h ^ (unsigned char)data[len - 1] << (8 * (len - 1)), len - 1);
        }

        static constexpr uint32_t readAsInt(const char* key) {
            return key[0] | ((uint32_t)(key[1]) << 8) | ((uint32_t)(key[2]) << 16) | ((uint32_t)(key[3]) << 24);
        }
        static constexpr uint32_t loop(const char* key, int len, uint32_t h) {

            return (len >= 4) ?
                loop(key + 4, len - 4, loopInt(readAsInt(key), h)) :
                ((len > 0) ? leftOver(key, h, len) : h);
        }

};
