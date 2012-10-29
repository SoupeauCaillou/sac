/*
	This file is part of Heriswap.

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
#include <libintl.h>
#include <locale.h>
#endif

#include <iostream> // à virer

#include <tinyxml2.h>

#include "../../base/Log.h"

void LocalizeAPILinuxImpl::init() {
	#ifndef EMSCRIPTEN
	char* lang = strdup(getenv("LANG"));
	lang[2] = '\0';
	
	std::string filename = DATADIR;
	filename += "../res/values";
	
	if (lang != "en")
		filename += "-" + std::string(lang);
	filename += "/strings.xml";
	
	tinyxml2::XMLDocument doc;
	
	if (doc.LoadFile(filename.c_str())) {
		LOGE("can't open xml file %s\n", filename.c_str());
		return;
	}

	tinyxml2::XMLHandle hDoc(&doc);
	tinyxml2::XMLElement * pElem;

	pElem = hDoc.FirstChildElement().ToElement();
	tinyxml2::XMLHandle hRoot(pElem);

	pElem = hRoot.FirstChildElement().ToElement();
	for (pElem; pElem; pElem = pElem->NextSiblingElement()) {
		_idToMessage[pElem->Attribute("name")] = pElem->GetText();
	}
	
	#endif

}

#define _(STRING)    gettext(STRING)

std::string LocalizeAPILinuxImpl::text(const std::string& s, const std::string& spc) {
#ifdef EMSCRIPTEN
	return _(spc.c_str());
#else
	return _idToMessage[s];
#endif
}
