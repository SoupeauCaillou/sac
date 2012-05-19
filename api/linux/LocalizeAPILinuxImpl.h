#pragma once

#include "../LocalizeAPI.h"

class LocalizeAPILinuxImpl : public LocalizeAPI {
	public:
		void init();
		std::string text(const std::string& s, const std::string& spc);
};
