#pragma once

#include "../NameInputAPI.h"
#include "../../base/EntityManager.h"

class NameInputAPILinuxImpl : public NameInputAPI {
    public:
        void show();
        bool done(std::string& name);
        void hide();

        Entity title, nameEdit, background;
        bool textIsReady;
};
