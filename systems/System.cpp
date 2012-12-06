#include "System.h"

std::map<std::string, ComponentSystem*> ComponentSystem::registry;

std::vector<std::string> ComponentSystem::registeredSystemNames() {
	std::vector<std::string> result;
	result.reserve(registry.size());
	for (std::map<std::string, ComponentSystem*>::iterator it=registry.begin();
		it!=registry.end();
		++it) {
		result.push_back(it->first);
	}
	return result;
}
