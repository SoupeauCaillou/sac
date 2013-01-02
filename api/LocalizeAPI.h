#pragma once

#include <string>

class LocalizeAPI {
	public :
		virtual std::string text(const std::string& s, const std::string& spc = "") = 0;

		virtual void changeLanguage(const std::string& s) = 0;
};
