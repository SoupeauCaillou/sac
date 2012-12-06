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

	if (strncmp(lang, "en", 2) != 0)
		filename += "-" + std::string(lang);
	filename += "/strings.xml";

	LOGI("'%s' => %s", lang, filename.c_str());
	free(lang);

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
	for (; pElem; pElem = pElem->NextSiblingElement()) {
        std::string s = pElem->GetText();
        while (s.find("\\n") != std::string::npos) {
            s.replace(s.find("\\n"), 2, "\n");
        }
        s.erase(s.begin());
        s.erase(s.end() - 1);
        _idToMessage[pElem->Attribute("name")] = s;
	}

	#endif

}

#define _(STRING)    gettext(STRING)

std::string LocalizeAPILinuxImpl::text(const std::string& s, const std::string& spc __attribute__((unused))) {
#ifdef EMSCRIPTEN
	return _(spc.c_str());

#else
	return _idToMessage[s];
#endif
}
