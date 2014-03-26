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

#include "api/AssetAPI.h"
#include <string>
#include <sstream>
#include "base/Log.h"
#include "util/MurmurHash.h"

class DataFileParser {
    public:
        static const std::string GlobalSection;

        DataFileParser();
        ~DataFileParser();

        // If you don't want to load a FileBuffer but only a DataFileParser, you can create init it
        // using this function
        void init();

        bool load(const FileBuffer& fb, const std::string& context);
        void unload();

        template <class T>
        int get(const std::string& section, const std::string& var, T* out, const int count = 1, bool warnIfNotFound = true) const;

        template <class T>
        int get(const std::string& section, hash_t var, T* out, const int count = 1, bool warnIfNotFound = true) const;

        template <class T>
        int get(const std::string& section, unsigned index, std::string& varName, T* out, const int count = 1) const;

        bool remove(const std::string& section, const std::string& var);

        unsigned sectionSize(const std::string& section) const;
        bool hasSection(const std::string& section) const;

        hash_t getModifier(const std::string& section, hash_t var) const;

        void defineVariable(const std::string& name, const std::string& value);
        int getSubStringCount(const std::string& section, const std::string& var) const;
        int getSubStringCount(const std::string& section, hash_t id) const;

        template <class T>
        void set(const std::string& section, const std::string& var, T* value, const int count = 1);

    private:
        bool keyValue(const std::string& section, const std::string& var, bool warnIfNotFound, std::string& value) const;
        bool indexValue(const std::string& section, unsigned index, std::string& varName, std::string& value) const;
        bool hashValue(const std::string& section, hash_t var, bool warnIfNotFound, std::string& value) const;

        template <class T>
        int parse(const std::string& value, T* out, int count = 1, bool warnIfNotFound = true) const;

        void put(const std::string& section, const std::string& var, const std::string& value);

    public:
        int determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes, bool warnIfNotFound) const;
        std::string replaceVariables(const std::string& str) const;

    private:
        struct DataFileParserData;
        DataFileParserData* data;
        std::string context;

    friend std::ostream & operator<<(std::ostream & o, const DataFileParser & dfp);
};

std::ostream & operator<<(std::ostream & o, const DataFileParser & dfp);

#define MAX_ELEMENTS 50

template <class T>
int DataFileParser::parse(const std::string& value, T* out, int count, bool warnIfNotFound) const {
    LOGF_IF(count > MAX_ELEMENTS, count << " elements not supported");

    size_t endIndexes[MAX_ELEMENTS];

    count = determineSubStringIndexes(value, count, endIndexes, warnIfNotFound);

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
    return count;
}

template <class T>
int DataFileParser::get(const std::string& section, const std::string& var, T* out, const int count, bool warnIfNotFound)  const{
    LOGF_IF(count <= 0, "Invalid 'count' param");
    // Retrieve value
    std::string val;
    if (!keyValue(section, var, warnIfNotFound, val))
        return false;

    return parse(val, out, count, warnIfNotFound);
}

template <class T>
int DataFileParser::get(const std::string& section, hash_t var, T* out, const int count, bool warnIfNotFound)  const{
    LOGF_IF(count <= 0, "Invalid 'count' param");
    // Retrieve value
    std::string val;
    if (!hashValue(section, var, warnIfNotFound, val))
        return false;

    return parse(val, out, count, warnIfNotFound);
}

template <>
inline int DataFileParser::get(const std::string& section, const std::string& var, char* out, const int maxCount, bool warnIfNotFound)  const{
    LOGF_IF(maxCount <= 0, "Invalid 'count' param");
    // Retrieve value
    std::string val;
    if (!keyValue(section, var, warnIfNotFound, val))
        return false;

    strncpy(out, val.c_str(), maxCount);
    out[maxCount - 1] = 0;

    return strlen(out);
}



template <class T>
int DataFileParser::get(const std::string& section, unsigned index, std::string& varName, T* out, const int count)  const{
    LOGF_IF(count <= 0, "Invalid 'count' param");
    // Retrieve value
    std::string val;
    if (!indexValue(section, index, varName, val))
        return false;

    return parse(val, out, count, true);
}

template <class T>
void DataFileParser::set(const std::string& section, const std::string& var, T* value, const int count) {
    std::stringstream ss;
    for (int i=0; i<count; i++) {
        if (i > 0)
            ss << ", ";
        ss << value[i];
    }
    put(section, var, ss.str());
}
