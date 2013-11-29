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



#if SAC_INGAME_EDITORS

#include <string>
#include <map>

#include <AntTweakBar.h>


class DebugConsole {
    public:
        //called in LevelEditor, creating "DebugConsole" icon in Ant
        void initTW();

        static DebugConsole & Instance();

        //Function without args (storingPlace is unused - should be 0)
        static void RegisterMethod(const std::string & functionName, void (*callback)(void*),
            void* storingPlace = 0);

        //user can choose the argument value of the specified type (see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twtype for the whole list)
        //functionName: string to be displayed in Ant TW bar
        //callback: function to be called when clicking the function in Ant
        //argumentName: string to be displayed in Ant TW bar (for the argument)
        //type: the argument type
        //      (bool=TW_TYPE_BOOLCPP, string=TW_TYPE_STDSTRING, float=TW_TYPE_FLOAT, ...
        //      see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twtype for the whole list)
        //storingPlace: where to store the argument value (should be a static or allocated variable)
        static void RegisterMethod(const std::string & functionName, void (*callback)(void*),
            const std::string & argumentName, TwType type, void* storingPlace);

        //user can choose the argument in a enum list
        //functionName: string to be displayed in Ant TW bar
        //callback: function to be called when clicking the function in Ant
        //argumentName: string to be displayed in Ant TW bar (for the argument)
        //availableArgs: enum list of possible values
        //availableArgsSize: size of the enum (note: you can use sizeof(availableArgs)/sizeof(TWEnumVal))
        //storingPlace: where to store the argument value (should be a static or allocated variable)
        static void RegisterMethod(const std::string & functionName, void (*callback)(void*),
            const std::string & argumentName, TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace);

    private:
        DebugConsole() {}
        ~DebugConsole() {}

    private:
        std::map<std::string, void (*)(void*)> name2callback;

        TwBar* bar;
};

#endif
