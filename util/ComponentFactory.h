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

class DataFileParser;
#include <string>
#include <vector>
class IProperty;
#include "systems/opengl/EntityTemplateLibrary.h"
class LocalizeAPI;

class ComponentFactory {
    public:
    	/*static int build(const DataFileParser& data,
    		const std::string& section,
    		const std::vector<IProperty*>& properties, void* component);*/

        static int build(const std::string& context, const DataFileParser& dfp,
            hash_t section,
            const std::vector<IProperty*>& properties, EntityTemplate& templ);

        static void applyTemplate(Entity e, void* comp, const PropertyNameValueMap& propValueMap, const std::vector<IProperty*>& properties, LocalizeAPI* locAPI);
};
