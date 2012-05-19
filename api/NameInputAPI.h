#pragma once

#include <string>

class NameInputAPI {
    public:
        virtual void show()=0;
        virtual bool done(std::string& name)=0;
        virtual void hide()=0;
};
