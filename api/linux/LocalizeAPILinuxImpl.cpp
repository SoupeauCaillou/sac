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



#include "LocalizeAPILinuxImpl.h"


#include "base/Log.h"
#if SAC_EMSCRIPTEN
#include <emscripten.h>
#elif SAC_WINDOWS
#include <Windows.h>
#else
#include <libintl.h>
#endif


#include <locale.h>
#include <tinyxml2.h>
#include "base/Log.h"
#include "api/AssetAPI.h"

#include <algorithm>
#include <sstream>

//return user locale (DE, EN, FR, etc.)
static std::string getLocaleInfo() {
#if SAC_WINDOWS
    WCHAR szISOLang[5] = {0};
    WCHAR szISOCountry[5] = {0};

    ::GetLocaleInfo(LOCALE_USER_DEFAULT,
                    LOCALE_SISO639LANGNAME,
                    (LPSTR)szISOLang,
                    sizeof(szISOLang) / sizeof(WCHAR));

    ::GetLocaleInfo(LOCALE_USER_DEFAULT,
                    LOCALE_SISO3166CTRYNAME,
					(LPSTR)szISOCountry,
                    sizeof(szISOCountry) / sizeof(WCHAR));
    
	std::wstring ws(szISOCountry);

	std::string lang((const char*)&ws[0], sizeof(wchar_t)/sizeof(char)*ws.size());
#elif SAC_EMSCRIPTEN
    std::string lang = emscripten_run_script_string( "navigator.language;" );
    lang.resize(2);
#else
    std::string lang(getenv("LANG"));
    //cut part after the '_' underscore
    lang.resize(2);
#endif

    //convert to lower case
    transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

    LOGI("Using locale: '" << lang << "'");
    return lang;
}

int LocalizeAPILinuxImpl::init(AssetAPI* assetAPI) {
    //first, clean the map
    _idToMessage.clear();

#if SAC_DARWIN

#else
    //first parse the english version
    parseXMLfile(assetAPI, "values/strings.xml");

#if SAC_EMSCRIPTEN
    // parsing values-fr crashes the app
    return 0;
#endif
    //then the user locale default, if different
    std::string lang = getLocaleInfo();
    if (lang != "en") {
        parseXMLfile(assetAPI, std::string("values-") + lang + "/strings.xml");
    }
#endif    
    return 0;
}

#if !SAC_DARWIN
void LocalizeAPILinuxImpl::parseXMLfile(AssetAPI* assetAPI, const std::string & filename) {
#if SAC_EMSCRIPTEN
    LOGI("ParseXML: " << filename);
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

        _idToMessage[key] = value;
    }

#else
    std::stringstream n;
    n << "../res/" << filename;
    FileBuffer fb = assetAPI->loadAsset(n.str());

    tinyxml2::XMLDocument doc;
    doc.Parse((const char*)fb.data);

    tinyxml2::XMLHandle hDoc(&doc);
    tinyxml2::XMLElement * pElem;

    pElem = hDoc.FirstChildElement().ToElement();
    tinyxml2::XMLHandle hRoot(pElem);

    for (pElem = hRoot.FirstChildElement().ToElement(); pElem;
    pElem = pElem->NextSiblingElement()) {

        std::string s = pElem->GetText();

        // replace new line in strings by a real new line
        while (s.find("\\n") != std::string::npos) {
            s.replace(s.find("\\n"), 2, "\n");
        }

        // and delete escape character before quote
        if (s.find("\"") == 0) {
            s = s.substr(1, s.length() - 2);
        }

        // then save it in the key
        _idToMessage[pElem->Attribute("name")] = s;
        LOGV(1, "'" << _idToMessage[pElem->Attribute("name")] << "' = '" << s << "'");
    }
    LOGI("Found " << _idToMessage.size() << " localized strings in file '" 
        << filename << "'. ");
        

    delete[] fb.data;
#endif
}
#endif

std::string LocalizeAPILinuxImpl::text(const std::string& s) {
    auto it = _idToMessage.find(s);
    LOGV(2, "Request localisation for: '" << s << "'");
    if (it == _idToMessage.end()) {
        LOGE_EVERY_N(60, "'" << s << "' is not a valid localizable ID");
        return "INVALID-" + s + "-ID";
    } else {
        return (*it).second;
    }
}
