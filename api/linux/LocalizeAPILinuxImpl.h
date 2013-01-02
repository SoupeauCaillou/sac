#pragma once

#include "../LocalizeAPI.h"

#include <map>

class LocalizeAPILinuxImpl : public LocalizeAPI {
	private:
		std::map<std::string, std::string> _idToMessage;
	public:
		int init(const std::string & lang);
		std::string text(const std::string& s, const std::string& spc);
		void changeLanguage(const std::string& s);

};
