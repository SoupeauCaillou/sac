#pragma once

#include <string>

//Get a string from user, platform dependent
class StringInputAPI {
    public:
        virtual void askUserInput(const std::string& initial = "", const int imaxSize = 10) = 0;
        virtual void cancelUserInput() = 0;
        //set string to the current input state and return true if user pressed 'enter'
        virtual bool done(std::string & entry) = 0;

        virtual int eventSDL(const void*) { return 0; }
};
