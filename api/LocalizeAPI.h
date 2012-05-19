#pragma once

#include <string>

class LocalizeAPI {
	public :
		virtual void init() = 0;
		virtual std::string text(const std::string& s) = 0;
};
