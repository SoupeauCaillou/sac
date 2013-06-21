#pragma once

#include "api/StringInputAPI.h"

class StringInputAPISDLImpl : public StringInputAPI {
    public:
        StringInputAPISDLImpl() : textIsReady(true) {}

        void askUserInput(const std::string& initial, const int imaxSize);
        void cancelUserInput();
        bool done(std::string & entry);

        int eventSDL(const void* event);
    private:
        bool textIsReady;
        std::string currentText;
        int maxSize;

};
