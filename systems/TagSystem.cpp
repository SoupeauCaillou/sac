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

#include "TagSystem.h"
#include "util/SerializerProperty.h"

INSTANCE_IMPL(TagSystem);

TagSystem::TagSystem() : ComponentSystemImpl<TagComponent>(HASH("Tag", 0x5fd02ba7)) {
    TagComponent tc;
    componentSerializer.add(new Property<int>(HASH("tags", 0), OFFSET(tags, tc)));
    for (int i=0; i<MAX_TAGS; i++) {
        tags[i] = 0;
    }
}

void TagSystem::DoUpdate(float ) { }

int8_t TagSystem::assignNextFreeTags(hash_t tag) {
    for (int i=0; i<MAX_TAGS; i++) {
        if (tags[i] == 0) {
            tags[i] = tag;
            return i;
        }
    }
    LOGE("Coulnd't find an empty slot for tag " << INV_HASH(tag));
    return -1;
}
