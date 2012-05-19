#pragma once

#include "../NameInputAPI.h"

class NameInputAPILinuxImpl : public NameInputAPI {
    public:
        void show();
        bool done(std::string& name);
        void hide();
};