#pragma once

#include "api/AssetAPI.h"
#include <string>
#include <sstream>
#include <glog/logging.h>

class DataFileParser {
    public:
        static const std::string GlobalSection;

        DataFileParser();
        ~DataFileParser();

        bool load(const FileBuffer& fb);
        void unload();

        template <class T>
        bool get(const std::string& section, const std::string& var, T* out, const int count = 1, bool warnIfNotFound = true);
        
        template <class T>
        bool get(const std::string& section, unsigned index, std::string& varName, T* out, const int count = 1);

        unsigned sectionSize(const std::string& section) const;

        void defineVariable(const std::string& name, const std::string& value);        

    private:
        const std::string& keyValue(const std::string& section, const std::string& var, bool warnIfNotFound) const;
        const std::string& indexValue(const std::string& section, unsigned index, std::string& varName) const;

        template <class T>
        bool parse(const std::string& value, T* out, const int count = 1);

    public:
        bool determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes);
        std::string replaceVariables(const std::string& str) const;

    private:
        struct DataFileParserData;
        DataFileParserData* data;
};

#define MAX_ELEMENTS 10

template <class T>
bool DataFileParser::parse(const std::string& value, T* out, const int count) {
	LOG_IF(FATAL, count > MAX_ELEMENTS)  << count << " elements not supported";
		
	size_t endIndexes[MAX_ELEMENTS];
    
	if (!determineSubStringIndexes(value, count, endIndexes))
        return false;

    size_t startIndex = 0;
    for (int i=0; i<count; i++) {
        size_t st = startIndex;
        size_t len = endIndexes[i] - startIndex + 1;
        // trim begin/end
        while (value[st] == ' ' || value[st] == '\t') { st++; len--; }
        while (value[st + len - 1] == ' ' || value[st + len - 1] == '\t') { len--; }
        std::string str = replaceVariables(value.substr(st, len));
        std::istringstream iss(str);
        startIndex = endIndexes[i] + 2;
        iss >> out[i];
    }
    return true;
}

template <class T>
bool DataFileParser::get(const std::string& section, const std::string& var, T* out, const int count, bool warnIfNotFound) {
    // Retrieve value
    const std::string& val = keyValue(section, var, warnIfNotFound);
    if (val.empty())
        return false;

    return parse(val, out, count);
}

template <class T>
bool DataFileParser::get(const std::string& section, unsigned index, std::string& varName, T* out, const int count) {
    // Retrieve value
    const std::string& val = indexValue(section, index, varName);
    if (val.empty())
        return false;

    return parse(val, out, count);
}
