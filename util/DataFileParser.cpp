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



#include "DataFileParser.h"
#include <map>
#include <algorithm>

struct Section {
    std::map<std::string, std::string> keyValues;
    std::map<hash_t, std::string> hashValues;
    std::map<hash_t, hash_t> hashModifiers;
};

struct DataFileParser::DataFileParserData {
    Section global;
    std::map<hash_t, Section*> sections;
    std::map<std::string, std::string> variables;

    ~DataFileParserData() {
        for(auto it=sections.begin();
            it!=sections.end(); ++it) {
            delete it->second;
        }
        sections.clear();
    }
    bool selectSectionByName(hash_t name, const Section** sectPtr, bool warnIfNotFound = true) const {
        if (name == GlobalSection) {
            *sectPtr = &global;
        } else {
            auto it = sections.find(name);
            if (it == sections.end()) {
                LOGW_IF(warnIfNotFound, "Cannot find section '" << INV_HASH(name) << "'");
                return false;
            }
            *sectPtr = it->second;
        }
        return true;
    }

    bool selectSectionByName(hash_t name, Section** sectPtr, bool warnIfNotFound = true) {
        if (name == GlobalSection) {
            *sectPtr = &global;
        } else {
            auto it = sections.find(name);
            if (it == sections.end()) {
                LOGW_IF(warnIfNotFound, "Cannot find section '" << INV_HASH(name) << "'");
                return false;
            }
            *sectPtr = it->second;
        }
        return true;
    }
};

DataFileParser::DataFileParser() : data(0) {

}

DataFileParser::~DataFileParser() {
    unload();
}

void DataFileParser::init() {
    if (data)
        delete data;

    data = new DataFileParserData();
}


bool DataFileParser::load(const FileBuffer& fb, const std::string& pContext) {
    LOGT_EVERY_N(100, "Add support for single-file->multi-file splitting using: ---8< pseudo-file-name ---");
    if (fb.size == 0)
        return false;
    context = pContext;
    if (!data)
        data = new DataFileParserData();

    Section* currentSection = &data->global;
    int size = fb.size;
    if (fb.data[size-1] == '\0') size--;

    std::stringstream f(std::string((const char*)fb.data, size), std::ios_base::in);

    std::string s("");
    while (getline(f, s)) {
        if (s.find_first_not_of("\t\n ") == std::string::npos)
            continue;
        if (s[0] == '#')
            continue;

        if (s[0] == '[') {
            // start new section
            std::string _section = s.substr(1, s.find(']') - 1);
            hash_t section = Murmur::RuntimeHash(_section.c_str());

            auto it = data->sections.find(section);
            if (it == data->sections.end()) {
                currentSection = new Section;
                data->sections.insert(std::make_pair(section, currentSection));
            } else {
                currentSection = it->second;
            }
        } else {
            // first '='
            int sep = s.find('=');

            if (sep == -1) {
                LOGE("Expecting to found '=' character in '" << s << "' but did NOT!");
                continue;
            }
            int st = 0, len = sep - st, end = s.size() - 1;
            while (s[st] == ' ' || s[st] == '\t') { st++; len--; }
            while (s[st + len - 1] == ' ' || s[st + len - 1] == '\t') len--;
            sep++;
            while (s[sep] == ' ' || s[sep] == '\t') sep++;
            while (s[end] == ' ' || s[end] == '\t') end--;

            std::string key(s.substr(st, len));
            std::string value(s.substr(sep, end));
            currentSection->keyValues.insert(std::make_pair(key, value));

            // Try something different
            const auto pct = key.find('%');
            if (pct == std::string::npos) {
                currentSection->hashValues.insert(std::make_pair(Murmur::RuntimeHash(key.c_str()), value));
            } else {
                hash_t h = Murmur::RuntimeHash(key.substr(0, pct).c_str());
                currentSection->hashValues.insert(std::make_pair(h, value));
                hash_t hm = Murmur::RuntimeHash(key.substr(pct + 1, std::string::npos).c_str());
                currentSection->hashModifiers.insert(std::make_pair(h, hm));
            }
        }
    }

    return true;
}

void DataFileParser::put(const std::string& section, const std::string& var, const std::string& value) {
    Section* sectPtr = 0;
    hash_t id = Murmur::RuntimeHash(section.c_str());
    if (!data->selectSectionByName(id, &sectPtr, false)) {
        sectPtr = new Section;
        data->sections.insert(std::make_pair(id, sectPtr));
    }
    sectPtr->keyValues[var] = value;
    sectPtr->hashValues[Murmur::RuntimeHash(var.c_str())] = value;
}

void DataFileParser::unload() {
    if (data)
        delete data;
    data = 0;
}

bool DataFileParser::keyValue(hash_t section, const std::string& var, bool LOG_USAGE_ONLY(warnIfNotFound), std::string& out) const {
    if (!data) {
        LOGE("No data loaded before requesting key value : " << section << '/' << var);
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    auto jt = sectPtr->keyValues.find(var);
    if (jt == sectPtr->keyValues.end()) {
        LOGE_IF(warnIfNotFound, context << ": cannot find var '" << var << "' in section '" << section << "'");
        return false;
    }
    out = jt->second;
    return true;
}

bool DataFileParser::hashValue(hash_t section, hash_t var, bool warnIfNotFound, std::string& out) const {
    if (!data) {
        LOGE("No data loaded before requesting key value : " << section << '/' << var);
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr, warnIfNotFound)) {
        return false;
    }
    auto jt = sectPtr->hashValues.find(var);
    if (jt == sectPtr->hashValues.end()) {
        LOGE_IF(warnIfNotFound, context << ": cannot find var '" << INV_HASH(var) << "' in section '" << INV_HASH(section) << "'");
        return false;
    }
    out = jt->second;
    return true;
}

hash_t DataFileParser::getModifier(hash_t section, hash_t var) const {
    if (!data) {
        LOGE("No data loaded before requesting key value : " << section << '/' << var);
        return 0;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return 0;
    }
    auto jt = sectPtr->hashModifiers.find(var);
    if (jt == sectPtr->hashModifiers.end()) {
        return 0;
    }
    return jt->second;
}

bool DataFileParser::remove(hash_t section, const std::string& var) {
    if (!data) {
        LOGE("No data loaded before requesting removal of key value : " << section << '/' << var);
        return false;
    }
    Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    auto it = sectPtr->keyValues.find(var);
    auto jt = sectPtr->hashValues.find(Murmur::RuntimeHash(var.c_str()));

    if (it == sectPtr->keyValues.end() || jt == sectPtr->hashValues.end()) {
        LOGE(context << ": cannot find var to remove '" << var << "' in section '" << section << "'");
        return false;
    }
    sectPtr->keyValues.erase(it);
    sectPtr->hashValues.erase(jt);

    return true;
}

bool DataFileParser::indexValue(hash_t section, unsigned index, std::string& varName, std::string& value) const {
    if (!data) {
        LOGE("No data loaded before requesting section " << section << " index " << index);
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    if (sectPtr->keyValues.size() <= index) {
        LOGE(context << ": requesting index : " << index << " in section " << section << ", which only contains " << sectPtr->keyValues.size() << " elements");
        return false;
    }
    auto jt = sectPtr->keyValues.begin();
    for (unsigned i=0; i<index; i++) jt++;
    varName = jt->first;
    value = jt->second;
    return true;
}

bool DataFileParser::hasSection(hash_t section) const {
    if (!data) {
        LOGE("No data loaded before requesting section size : " << section);
        return false;
    }
    if (section == GlobalSection)
        return true;
    return data->sections.find(section) != data->sections.end();
}

unsigned DataFileParser::sectionSize(hash_t section) const {
    if (!data) {
        LOGE("No data loaded before requesting section size : " << section);
        return 0;
    }
    if (section == GlobalSection)
        return data->global.keyValues.size();
    auto it = data->sections.find(section);
    if (it == data->sections.end()) {
        LOGE(context << ": cannot find section '" << section << "'");
        return 0;
    }
    return it->second->keyValues.size();
}

int DataFileParser::determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes, bool LOG_USAGE_ONLY(warnIfNotFound)) const{
    // Determine substring indexes
    outIndexes[count - 1] = str.size() - 1;
    size_t index = 0;
    for (int i=0; i<count-1; i++) {
        index = str.find(',', index);
        if (index == std::string::npos) {
            LOGW_IF(warnIfNotFound, context << ": entry '" << str << "' does not contain '" << count << "' values");
            outIndexes[i] = str.size() - 1;
            return (i + 1);
        } else {
            outIndexes[i] = index - 1;
            LOGV(3, i << " = " << outIndexes[i]);
            index += 2;
        }
    }
    LOGV(3, (count-1) << " = " << outIndexes[count-1] << "* (" << str << ')');
    return count;
}

int DataFileParser::getSubStringCount(hash_t section, const std::string& var) const {
    std::string s;
    if (keyValue(section, var, false, s)) {
        // let's count ','
        int splits = 1;
        std::for_each(s.begin(), s.end(), [&splits](char c) -> void { if (c ==',') splits++;});
        return splits;
    } else {
        return -1;
    }
}

int DataFileParser::getSubStringCount(hash_t section, hash_t id) const {
    std::string s;
    if (hashValue(section, id, false, s)) {
        // let's count ','
        int splits = 1;
        std::for_each(s.begin(), s.end(), [&splits](char c) -> void { if (c ==',') splits++;});
        return splits;
    } else {
        return -1;
    }
}

void DataFileParser::defineVariable(const std::string& name, const std::string& value) {
    if (!data) {
        LOGE("No data loaded before setting variable");
    } else {
        std::stringstream v;
        v << '$' << name;
        data->variables[v.str()] = value;
    }
}

std::string DataFileParser::replaceVariables(const std::string& str) const {
    std::string result(str);
    for (auto it=data->variables.begin(); it!=data->variables.end(); ++it) {
        size_t idx;
        while ((idx = result.find(it->first)) != std::string::npos) {
            result = result.replace(idx, it->first.size(), it->second);
        }
    }
    return result;
}

std::ostream & operator<<(std::ostream & o, const DataFileParser & dfp) {
    LOGT("Variables not handled!");

    //Global section first
    for (auto & kv : dfp.data->global.keyValues) {
        o << kv.first << "=" << kv.second << std::endl;
    }

    for (auto & section : dfp.data->sections) {
        o << "[" << section.first << "]" << std::endl;
        for (auto & kv : section.second->keyValues) {
            o << kv.first << "=" << kv.second << std::endl;
        }
        o << std::endl;
    }

    return o;
}
