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
        bool get(const std::string& section, const std::string& var, T* out, const int count = 1);
        
        template <class T>
        bool get(const std::string& section, unsigned index, std::string& varName, T* out, const int count = 1);

        unsigned sectionSize(const std::string& section) const;
        

    private:
        const std::string& keyValue(const std::string& section, const std::string& var) const;
        const std::string& indexValue(const std::string& section, unsigned index, std::string& varName) const;

        template <class T>
        bool parse(const std::string& value, T* out, const int count = 1);

    public:
        bool determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes);

    private:
        struct DataFileParserData;
        DataFileParserData* data;
};

template <class T>
bool DataFileParser::parse(const std::string& value, T* out, const int count) {
    size_t endIndexes[count];
    if (!determineSubStringIndexes(value, count, endIndexes))
        return false;

    size_t startIndex = 0;
    for (int i=0; i<count; i++) {
        size_t st = startIndex;
        size_t len = endIndexes[i] - startIndex + 1;
        // trim begin/end
        while (value[st] == ' ' || value[st] == '\t') { st++; len--; }
        while (value[st + len - 1] == ' ' || value[st + len - 1] == '\t') { len--; }
        std::string str = value.substr(st, len);
        std::istringstream iss(str);
        startIndex = endIndexes[i] + 2;
        iss >> out[i];
    }
    return true;
}

template <class T>
bool DataFileParser::get(const std::string& section, const std::string& var, T* out, const int count) {
    // Retrieve value
    const std::string& val = keyValue(section, var);
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
