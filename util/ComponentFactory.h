#pragma once

class DataFileParser;
#include <string>
#include <vector>
class IProperty;
#include "systems/opengl/EntityTemplateLibrary.h"

class ComponentFactory {
    public:
    	/*static int build(const DataFileParser& data,
    		const std::string& section,
    		const std::vector<IProperty*>& properties, void* component);*/

        static int build(const DataFileParser& dfp,
            const std::string& section,
            const std::vector<IProperty*>& properties, EntityTemplate& templ);

        static void applyTemplate(void* comp, const PropertyNameValueMap& propValueMap, const std::vector<IProperty*>& properties);
};
