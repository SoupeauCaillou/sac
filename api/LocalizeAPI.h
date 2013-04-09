#pragma once

#include <string>

class LocalizeAPI {
	public :
		virtual std::string text(const std::string& s) = 0;
};
