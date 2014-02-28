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

const std::string DataFileParser::GlobalSection = "";

typedef std::map<std::string, std::string> Section;

struct DataFileParser::DataFileParserData {
    Section global;
    std::map<std::string, Section*> sections;
    std::map<std::string, std::string> variables;

    ~DataFileParserData() {
        for(std::map<std::string, Section*>::iterator it=sections.begin();
            it!=sections.end(); ++it) {
            delete it->second;
        }
        sections.clear();
    }
    bool selectSectionByName(const std::string& name, const Section** sectPtr) const {
        if (name == GlobalSection) {
            *sectPtr = &global;
        } else {
            std::map<std::string, Section*>::const_iterator it = sections.find(name);
            if (it == sections.end()) {
                LOGE("Cannot find section '" << name << "'");
                return false;
            }
            *sectPtr = it->second;
        }
        return true;
    }

    bool selectSectionByName(const std::string& name, Section** sectPtr) {
        if (name == GlobalSection) {
            *sectPtr = &global;
        } else {
            std::map<std::string, Section*>::iterator it = sections.find(name);
            if (it == sections.end()) {
                LOGE("Cannot find section '" << name << "'");
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
        if (s.empty())
            continue;
        if (s[0] == '#')
            continue;

        if (s[0] == '[') {
            // start new section
            std::string section = s.substr(1, s.find(']') - 1);

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
            currentSection->insert(std::make_pair(key, value));
        }
    }

    return true;
}

void DataFileParser::put(const std::string& section, const std::string& var, const std::string& value) {
    Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        sectPtr = new Section;
        data->sections.insert(std::make_pair(section, sectPtr));
    }
    (*sectPtr)[var] = value;
}

void DataFileParser::unload() {
    if (data)
        delete data;
    data = 0;
}

bool DataFileParser::keyValue(const std::string& section, const std::string& var, bool LOG_USAGE_ONLY(warnIfNotFound), std::string& out) const {
    if (!data) {
        LOGE("No data loaded before requesting key value : " << section << '/' << var);
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    std::map<std::string, std::string>::const_iterator jt = sectPtr->find(var);
    if (jt == sectPtr->end()) {
        LOGE_IF(warnIfNotFound, context << ": cannot find var '" << var << "' in section '" << section << "'");
        return false;
    }
    out = jt->second;
    return true;
}

bool DataFileParser::remove(const std::string& section, const std::string& var) {
    if (!data) {
        LOGE("No data loaded before requesting removal of key value : " << section << '/' << var);
        return false;
    }
    Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    auto it = sectPtr->find(var);
    if (it == sectPtr->end()) {
        LOGE(context << ": cannot find var to remove '" << var << "' in section '" << section << "'");
        return false;
    }
    sectPtr->erase(it);
    return true;
}

bool DataFileParser::indexValue(const std::string& section, unsigned index, std::string& varName, std::string& value) const {
    if (!data) {
        LOGE("No data loaded before requesting section " << section << " index " << index);
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    if (sectPtr->size() <= index) {
        LOGE(context << ": requesting index : " << index << " in section " << section << ", which only contains " << sectPtr->size() << " elements");
        return false;
    }
    Section::const_iterator jt = sectPtr->begin();
    for (unsigned i=0; i<index; i++) jt++;
    varName = jt->first;
    value = jt->second;
    return true;
}

bool DataFileParser::hasSection(const std::string& section) const {
    if (!data) {
        LOGE("No data loaded before requesting section size : " << section);
        return false;
    }
    if (section == GlobalSection)
        return true;
    return data->sections.find(section) != data->sections.end();
}

unsigned DataFileParser::sectionSize(const std::string& section) const {
    if (!data) {
        LOGE("No data loaded before requesting section size : " << section);
        return 0;
    }
    if (section == GlobalSection)
        return data->global.size();
    std::map<std::string, Section*>::const_iterator it = data->sections.find(section);
    if (it == data->sections.end()) {
        LOGE(context << ": cannot find section '" << section << "'");
        return 0;
    }
    return it->second->size();
}

bool DataFileParser::determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes, bool LOG_USAGE_ONLY(warnIfNotFound)) const{
    // Determine substring indexes
    outIndexes[count - 1] = str.size() - 1;
    size_t index = 0;
    for (int i=0; i<count-1; i++) {
        index = str.find(',', index);
        if (index == std::string::npos) {
            LOGE_IF(warnIfNotFound, context << ": entry '" << str << "' does not contain '" << count << "' values");
            return false;
        } else {
            outIndexes[i] = index - 1;
            LOGV(2, i << " = " << outIndexes[i]);
            index += 2;
        }
    }
    LOGV(2, (count-1) << " = " << outIndexes[count-1] << "* (" << str << ')');
    return true;
}

int DataFileParser::getSubStringCount(const std::string& section, const std::string& var) const {
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
    for (auto & kv : dfp.data->global) {
        o << kv.first << "=" << kv.second << std::endl;
    }

    for (auto & section : dfp.data->sections) {
        o << "[" << section.first << "]" << std::endl;
        for (auto & kv : *section.second) {
            o << kv.first << "=" << kv.second << std::endl;
        }
        o << std::endl;
    }

    return o;
}
