#pragma once

class DataFileParser;
#include <string>
#include <vector>
class IProperty;

class ComponentFactory {
    public:
    	static int build(const DataFileParser& data,
    		const std::string& section,
    		const std::vector<IProperty*>& properties, void* component);

};
