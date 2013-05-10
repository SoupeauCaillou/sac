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

    std::string s;
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

void DataFileParser::unload() {
    if (data)
        delete data;
    data = 0;
}

bool DataFileParser::keyValue(const std::string& section, const std::string& var, bool warnIfNotFound, std::string& out) const {
    if (!data) {
        LOGE("No data loaded before requesting key value : " << section << '/' << var)
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    std::map<std::string, std::string>::const_iterator jt = sectPtr->find(var);
    if (jt == sectPtr->end()) {
        LOGE_IF(warnIfNotFound, context << ": cannot find var '" << var << "' in section '" << section << "'")
        return false;
    }
    out = jt->second;
    return true;
}

bool DataFileParser::indexValue(const std::string& section, unsigned index, std::string& varName, std::string& value) const {
    if (!data) {
        LOGE("No data loaded before requesting section " << section << " index " << index)
        return false;
    }
    const Section* sectPtr = 0;
    if (!data->selectSectionByName(section, &sectPtr)) {
        return false;
    }
    if (sectPtr->size() <= index) {
        LOGE(context << ": requesting index : " << index << " in section " << section << ", which only contains " << sectPtr->size() << " elements")
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
        LOGE("No data loaded before requesting section size : " << section)
        return false;
    }
    if (section == GlobalSection)
        return true;
    return data->sections.find(section) != data->sections.end();
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
        LOGE(context << ": cannot find section '" << section << "'")
        return 0;
    }
    return it->second->size();
}

bool DataFileParser::determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes, bool warnIfNotFound) const{
    // Determine substring indexes
    outIndexes[count - 1] = str.size() - 1;
    size_t index = 0;
    for (int i=0; i<count-1; i++) {
        index = str.find(',', index);
        if (index == std::string::npos) {
            LOGE_IF(warnIfNotFound, context << ": entry '" << str << "' does not contain '" << count << "' values")
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
