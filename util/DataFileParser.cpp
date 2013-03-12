#include "DataFileParser.h"
#include <map>

struct Section {
    std::map<std::string, std::string> keyValues;
};

struct DataFileParser::DataFileParserData {
    std::map<std::string, Section> sections;
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

    std::map<std::string, Section>::iterator it = data->sections.end();

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
            it = data->sections.insert(std::make_pair(section, Section())).first;
        } else {
            if (it == data->sections.end()) {
                LOG(ERROR) << "No section before line: " << s;
                return false;
            }
            std::string key = s.substr(0, s.find('='));
            std::string value = s.substr(s.find('=') + 1);
            it->second.keyValues.insert(std::make_pair(key, value));
        }
    }
        
    return true;
}

void DataFileParser::unload() {
    if (data)
        delete data;
    data = 0;
}

const std::string& DataFileParser::keyValue(const std::string& section, const std::string& var) const {
    static const std::string empty = "";
    if (!data) {
        LOG(ERROR) << "No data loaded before requesting key value : " << section << '/' << var;
        return empty;
    }
    std::map<std::string, Section>::const_iterator it = data->sections.find(section);
    if (it == data->sections.end()) {
        LOG(ERROR) << "Cannot find section '" << section << "'";
        return empty;
    }
    std::map<std::string, std::string>::const_iterator jt = it->second.keyValues.find(var);
    if (jt == it->second.keyValues.end()) {
        LOG(ERROR) << "Cannot find var '" << var << "' in section '" << section << "'";
        return empty;
    }
    return jt->second;
}

bool DataFileParser::determineSubStringIndexes(const std::string& str, int count, size_t* outIndexes) {
    // Determine substring indexes
    outIndexes[count - 1] = str.size() - 1;
    size_t index = 0;
    for (int i=0; i<count-1; i++) {
        index = str.find(',', index);
        if (index == std::string::npos) {
            LOG(ERROR) << "Entry '" << str << "' does not contain '" << count << "' values";
            return false;
        } else {
            outIndexes[i] = index - 1;
            VLOG(2) << i << " = " << outIndexes[i];
            index += 2;
        }
    }
    VLOG(2) << (count-1) << " = " << outIndexes[count-1] << "* (" << str << ')';
    return true;
}
