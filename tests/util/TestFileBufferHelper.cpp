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

#include "util/FileBufferHelper.h"
#include "api/AssetAPI.h"

static FileBuffer FB(const char* content) {
    FileBuffer fb;
    fb.data = (uint8_t*)content;
    fb.size = strlen(content);
    return fb;
}

TEST(TestOneLine) {
    FileBuffer fb = FB("abcde");
    FileBufferHelper helper;

    CHECK_EQUAL(0, strcmp("abcde", helper.line(fb)));
}

TEST(TestTwoLine) {
    FileBuffer fb = FB("abcde\nfghijk\n\n");
    FileBufferHelper helper;

    CHECK_EQUAL("abcde", std::string(helper.line(fb)));
    CHECK_EQUAL("fghijk", std::string(helper.line(fb)));
    CHECK_EQUAL("\0", std::string(helper.line(fb)));
    CHECK(NULL == helper.line(fb));
}
