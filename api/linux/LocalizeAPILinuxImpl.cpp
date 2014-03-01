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
#elif SAC_WINDOWS
#include <Windows.h>
#else
#include <libintl.h>
#include <locale.h>
#endif

#include <algorithm>

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
#else
    std::string lang(getenv("LANG"));
    //cut part after the '_' underscore
    lang.resize(2);
#endif

    //convert to lower case
    transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
    return lang;
}

#if SAC_DARWIN || SAC_EMSCRIPTEN

int LocalizeAPILinuxImpl::init(AssetAPI*) {
	return 0;
}

#else

#include <tinyxml2.h>
#include "base/Log.h"
#include "api/AssetAPI.h"

int LocalizeAPILinuxImpl::init(AssetAPI* assetAPI) {

    std::string filename = "../res/values";
    std::string lang = getLocaleInfo();

    if (lang != "en") {
        filename += "-";
        filename += lang.c_str();
    }
    filename += "/strings.xml";
    LOGI("[INIT]Found language:" << lang << " -> " << filename);

    //first, clean the map
    _idToMessage.clear();

    FileBuffer fb = assetAPI->loadAsset(filename);

    tinyxml2::XMLDocument doc;
    doc.Parse((const char*)fb.data);

    tinyxml2::XMLHandle hDoc(&doc);
    tinyxml2::XMLElement * pElem;

    pElem = hDoc.FirstChildElement().ToElement();
    tinyxml2::XMLHandle hRoot(pElem);

    for (pElem = hRoot.FirstChildElement().ToElement(); pElem;
    pElem = pElem->NextSiblingElement()) {

        std::string s = pElem->GetText();

        while (s.find("\\n") != std::string::npos) {
            s.replace(s.find("\\n"), 2, "\n");
        }
        if (s.find("\"") == 0) {
            s = s.substr(1, s.length() - 2);
        }
        _idToMessage[pElem->Attribute("name")] = s;
        LOGV(1, "'" << _idToMessage[pElem->Attribute("name")] << "' = '" << s << "'");
    }
    LOGI("[INIT]Localize strings count: " << _idToMessage.size());

    delete[] fb.data;

    return 0;
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
