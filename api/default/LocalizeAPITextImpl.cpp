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



#include "LocalizeAPITextImpl.h"

#include "base/Log.h"

#if SAC_EMSCRIPTEN
#include <emscripten.h>
#endif

#include <locale.h>
#include "base/Log.h"
#include "api/AssetAPI.h"

#include <algorithm>
#include <sstream>

int LocalizeAPITextImpl::init(AssetAPI* assetAPI, const char * userLang, const char * defaultLang) {
    //first parse the English version
    readTXTFile(assetAPI, defaultTexts, defaultLang);

    #if SAC_EMSCRIPTEN
        LOGT("Still needed?");
        // parsing values-fr crashes the app
        return 0;
    #endif

    //then the user locale default, if different
    if (strcmp(userLang, defaultLang) != 0) {
        readTXTFile(assetAPI, userLanguageTexts, userLang);
    }
    return 0;
}

void LocalizeAPITextImpl::readTXTFile(AssetAPI* assetAPI, std::vector<std::string> & texts, const char * lang) {
    std::string filename = "strings/" + std::string(lang) + ".txt";
    #if SAC_EMSCRIPTEN
        LOGI("Fixme ParseXML: " << filename);
        // weeee
        std::string javascript (
                    "a = FS.readFile('/assets/FILENAME', { encoding: 'utf8' });"
            "parser = new DOMParser();"
            "doc = parser.parseFromString(a, 'text/xml');"
            "ssss = doc.getElementsByTagName('string');"
            "result = '';"
            "for (i=0; i<ssss.length; i++) {"
            "   result += ssss[i].attributes[0].value;"
            "   result += ',';"
            "   result += ssss[i].childNodes[0].nodeValue;"
            "   result += ',';"
            "}"
            "result;");
        javascript.replace(javascript.find("FILENAME"), strlen("FILENAME"), filename);

        std::string plop = emscripten_run_script_string (javascript.c_str());
        int next = 0;
        while(next < plop.length()) {
            // find key delimiter
            int end = plop.find(',', next);
            if (end == std::string::npos)
                break;

            std::string key = plop.substr(next, end - next);

            next = end + 1;
            // find value delimiter
            end = plop.find(',', next);
            // remove ""
            std::string value = plop.substr(next + 1, end - next - 2);
            next = end + 1;

            //_idToMessage[key] = value;
        }

    #else
        FileBuffer fb = assetAPI->loadAsset(filename);

        if (fb.size == 0) {
            LOGW("Cannot read '" << filename << "' localization file");
            return;
        }

        int size = fb.size;
        if (fb.data[size-1] == '\0') size--;


        std::string line;
        std::stringstream f(std::string((const char*)fb.data, size), std::ios_base::in);

        while (getline(f, line)) {
            // replace new line in strings by a real new line
            while (line.find("\\n") != std::string::npos) {
                line.replace(line.find("\\n"), 2, "\n");
            }

            // then save it in the key
            texts.push_back(line);
            LOGV(1, "'" << texts.size()-1 << "' = '" << line << "'");
        }
        LOGV(1, "Found " << texts.size() << " localized strings in file '"
            << filename << "'. ");

        delete[] fb.data;
    #endif
}

std::string LocalizeAPITextImpl::text(const std::string& s) {
    LOGV(2, "Request localization for: '" << s << "'");
    // do not even try to translate if we are using default language (release mode only)
    #if ! SAC_DEBUG
        if (userLanguageTexts.empty()) {
            return s;
        }
    #endif

    // otherwise find position in vector...
    unsigned pos = std::find(defaultTexts.begin(), defaultTexts.end(), s) - defaultTexts.begin();
    if (pos == defaultTexts.size()) {
        LOGE_EVERY_N(60, "'" << s << "' is not a valid localizable ID");
        return "INVALID-" + s + "-ID";
    }

    #if SAC_DEBUG
        if (userLanguageTexts.empty()) {
            return s;
        }
    #endif

    assert(userLanguageTexts.size() == defaultTexts.size());

    // and return translation
    return userLanguageTexts[pos];
}
