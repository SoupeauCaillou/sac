#if SAC_INGAME_EDITORS

#include "base/Log.h"

#include <string>
#include <map>

#include <AntTweakBar.h>


class DebugConsole {
    public:
        //called in LevelEditor, creating "DebugConsole" icon in Ant
        void initTW();

        static DebugConsole & Instance();

        //there is no argument
        static void registerMethodWithoutArg(const std::string & functionName, void (*callback)(void*));

        //user can choose the argument in a enum list
        //functionName: string to be displayed in Ant TW bar
        //argumentName: string to be displayed in Ant TW bar (for the argument)
        //callback: function to be called when clicking the function in Ant
        //availableArgs: enum list of possible values
        //availableArgsSize: size of the enum (note: you can use sizeof(availableArgs)/sizeof(TWEnumVal))
        //storingPlace: where to store the argument value (should be a static or allocated variable)
        static void registerMethodWithOneArg(const std::string & functionName, const std::string & argumentName,
            void (*callback)(void*), TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace);

        //user can choose the argument value of the specified type (see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twtype for the whole list)
        //functionName: string to be displayed in Ant TW bar
        //argumentName: string to be displayed in Ant TW bar (for the argument)
        //callback: function to be called when clicking the function in Ant
        //type: the argument type (bool, string, int, float, ... see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twtype for the whole list)
        //storingPlace: where to store the argument value (should be a static or allocated variable)
        static void registerMethodWithOneArg(const std::string & functionName, const std::string & argumentName,
            void (*callback)(void*), void* storingPlace, TwType type);

    private:
        DebugConsole() {}
        ~DebugConsole() {}

    private:
        std::map<std::string, void (*)(void*)> name2callback;

        TwBar* bar;
};

#endif
