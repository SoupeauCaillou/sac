#include "DataFileParser.h"
#include <map>

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
                LOGE("Cannot find section '" << name << "'")
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

bool DataFileParser::load(const FileBuffer& fb) {
    if (data)
        return false;
    if (fb.size == 0)
        return false;
    data = new DataFileParserData();

    Section* currentSection = &data->global;
    std::stringstream f(std::string((const char*)fb.data, fb.size), std::ios_base::in);

    std::string s;
    while (getline(f, s)) {
        if (s.empty())
            continue;
        if (s[0] == '#')
            continue;

        if (s[0] == '[') {
            // start new section
            std::string section = s.substr(1, s.find(']') - 1);
            currentSection = new Section;
            if (!data->sections.insert(std::make_pair(section, currentSection)).second) {
                LOGW("Duplicate section found: '" << section << "'. This is not supported")
            }
        } else {
            std::string key = s.substr(0, s.find('='));
            std::string value = s.substr(s.find('=') + 1);
            currentSection->insert(std::make_pair(key, value));
        }
    }
        
    return true;
}

void DataFileParser::unload() {
    if (data)
        delete data;
    data = 0;
}

const std::string& DataFileParser::keyValue(const std::string& section, const std::string& var, bool warnIfNotFound) const {
    static const std::string empty = "";
    if (!data) {
        LOGE("No data loaded before requesting key value : " << section << '/' << var)
        return empty;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return empty;
    }
    std::map<std::string, std::string>::const_iterator jt = sectPtr->find(var);
    if (jt == sectPtr->end()) {
        LOGE_IF(warnIfNotFound, "Cannot find var '" << var << "' in section '" << section << "'")
        return empty;
    }
    return jt->second;
}

const std::string& DataFileParser::indexValue(const std::string& section, unsigned index, std::string& varName) const {
    static const std::string empty = "";
    if (!data) {
        LOGE("No data loaded before requesting section " << section << " index " << index)
        return empty;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return empty;
    }
    if (sectPtr->size() <= index) {
        LOGE("Requesting index : " << index << " in section " << section << ", which only contains " << sectPtr->size() << " elements")
        return empty;
    }
    Section::const_iterator jt = sectPtr->begin();
    for (unsigned i=0; i<index; i++) jt++;
    varName = jt->first;
    return jt->second;
}

unsigned DataFileParser::sectionSize(const std::string& section) const {
    if (!data) {
        LOGE("No data loaded before requesting section size : " << section)
        return 0;
    }
    if (section == GlobalSection)
        return data->global.size();
    std::map<std::string, Section*>::const_iterator it = data->sections.find(section);
    if (it == data->sections.end()) {
        LOGE("Cannot find section '" << section << "'")
        return 0;
    }
    return it->second->size();
}

bool DataFileParser::determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes) {
    // Determine substring indexes
    outIndexes[count - 1] = str.size() - 1;
    size_t index = 0;
    for (int i=0; i<count-1; i++) {
        index = str.find(',', index);
        if (index == std::string::npos) {
            LOGE("Entry '" << str << "' does not contain '" << count << "' values")
            return false;
        } else {
            outIndexes[i] = index - 1;
            LOGV(2, i << " = " << outIndexes[i])
            index += 2;
        }
    }
    LOGV(2, (count-1) << " = " << outIndexes[count-1] << "* (" << str << ')')
    return true;
}

void DataFileParser::defineVariable(const std::string& name, const std::string& value) {
    if (!data) {
        LOGE("No data loaded before setting variable")
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
