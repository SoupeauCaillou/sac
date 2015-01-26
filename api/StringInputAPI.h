/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include <string>
#include <vector>

//Get a string from user, platform dependent
class StringInputAPI {
    public:
        virtual ~StringInputAPI() {}

        virtual void askUserInput(const std::string& initial = "", const int imaxSize = 10) = 0;
        virtual void cancelUserInput() = 0;
        //set string to the current input state and return true if user pressed 'enter'
        virtual bool done(std::string & entry) = 0;

        virtual void setNamesList(const std::vector<std::string> & names) = 0;

        virtual int eventSDL(const void*) { return 0; }
};
