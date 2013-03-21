/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    Heriswap is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Heriswap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "LocalizeAPILinuxImpl.h"
#ifndef EMSCRIPTEN
	#ifdef WINDOWS
	//TODO
	#else
	#include <libintl.h>
	#include <locale.h>
	#endif
#endif

#include <iostream> // à virer

#include <tinyxml2.h>

#ifdef WINDOWS
#include <base/Log.h>
#else
#include <glog/logging.h>
#endif
int LocalizeAPILinuxImpl::init(const std::string & lang) {
#ifdef DATADIR
    std::string filename = SAC_ASSETS_DIR;
    filename += "../res/values";

    if (strcmp(lang.c_str(),"en")) {
    //- if (lang != "en") {
        filename += "-";
        filename += lang.c_str();
    }
    filename += "/strings.xml";
    LOG(INFO) << lang << " -> " << filename;
#else
    std::string filename = "assets/strings.xml";
#endif

    //first, clean the map
    _idToMessage.clear();

    tinyxml2::XMLDocument doc;

    if (doc.LoadFile(filename.c_str())) {
        LOG(WARNING) << "can't open xml file " << filename;
        return -1;
    }

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
        VLOG(1) << "'" << _idToMessage[pElem->Attribute("name")] << "' = '" << s << "'";
    }
    LOG(INFO) << "Localize strings count: " << _idToMessage.size();

    return 0;
}

void LocalizeAPILinuxImpl::changeLanguage(const std::string& s) {
    init(s);
}

std::string LocalizeAPILinuxImpl::text(const std::string& s, const std::string&) {
    return _idToMessage[s];
}
