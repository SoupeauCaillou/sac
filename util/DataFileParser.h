#pragma once

#include "api/AssetAPI.h"
#include <string>
#include <sstream>
#include <glog/logging.h>

class DataFileParser {
    public:
        DataFileParser();
        ~DataFileParser();

        bool load(const FileBuffer& fb);
        void unload();

        template <class T>
        bool get(const std::string& section, const std::string& var, T* out, const int count = 1);

    private:
        const std::string& keyValue(const std::string& section, const std::string& var) const;
    public:
        bool determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes);

    private:
        struct DataFileParserData;
        DataFileParserData* data;
};

template <class T>
bool DataFileParser::get(const std::string& section, const std::string& var, T* out, const int count) {
    // Retrieve value
    const std::string& val = keyValue(section, var);
    if (val.empty())
        return false;

    size_t endIndexes[count];
    if (!determineSubStringIndexes(val, count, endIndexes))
        return false;

    size_t startIndex = 0;
    for (int i=0; i<count; i++) {
        size_t st = startIndex;
        size_t len = endIndexes[i] - startIndex + 1;
        // trim begin/end
        while (val[st] == ' ' || val[st] == '\t') { st++; len--; }
        while (val[st + len - 1] == ' ' || val[st + len - 1] == '\t') { len--; }
        std::string str = val.substr(st, len);
        VLOG(1) << "Section:'" << section << "', key:'" << var << "' index:" << (i+1) << "/" << count << " = '" << str << "'";
        VLOG(2) << "Initial start/len: " << startIndex << "/" << (endIndexes[i] - startIndex + 1) << ". Real start/len: " << st << "/" << len;
        std::istringstream iss(str);
        startIndex = endIndexes[i] + 2;
        iss >> out[i];
    }
    return true;
}