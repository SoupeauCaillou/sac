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

#include <UnitTest++.h>

#include "util/Random.h"

#include <iostream>
#include <vector>
#include <random>

TEST(IntRandomInInterval) {
    srand(time(NULL));
    int min_value = 1;
    int max_value = 14;
    bool fail = false;
    for (int i=0; i<10000; ++i) {
        int ret = Random::Int(min_value, max_value);
        if (ret > max_value || ret < min_value) {
            fail = true;
            break;
        }
    }
	CHECK_EQUAL(false, fail);
}

TEST(IntRandomRepartition) {
    int min_value = 0;
    int max_value = 10;
    int sum = 0;
    for (int i=0; i<10000; ++i) {
        sum += Random::Int(min_value, max_value);
    }

    CHECK_CLOSE(5, sum/10000.f, 0.1);
}

TEST(FloatRandomInInterval) {
    float min_value = 3.5f;
    float max_value = 14.032f;
    bool fail = false;
    for (int i=0; i<10000; ++i) {
        float ret = Random::Float(min_value, max_value);
        if (ret > max_value || ret < min_value) {
            fail = true;
            break;
        }
    }
    CHECK_EQUAL(false, fail);
}

TEST(FloatRandomRepartition) {
    float min_value = -1.f;
    float max_value = 1.0f;
    int sum = 0;
    
    for (int i=0; i<1000000; ++i) {
        float ret = Random::Float(min_value, max_value);
        sum += ret;
    }
    
    CHECK_CLOSE(0.0f, sum/1000000.f, 0.05);
}
