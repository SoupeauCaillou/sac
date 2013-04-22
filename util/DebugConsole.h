#if SAC_INGAME_EDITORS

#include "base/Log.h"

#include <string>
#include <map>

#include <AntTweakBar.h>


//use this method to add a method to the debug console with an enum argument
#define REGISTER_ONE_ARG(name, args)\
    static int name##Value = args[0].Value;\
    DebugConsole::Instance().registerMethodWithOneArg(#name, #args, &callback##name, args, sizeof(args)/sizeof(TwEnumVal), &name##Value);

//use this method to add a method to the debug console without arg
#define REGISTER_NO_ARG(name)\
    DebugConsole::Instance().registerMethodWithoutArg(#name, &callback##name);



class DebugConsole {
    public:
        static DebugConsole & Instance();

        static void registerMethodWithOneArg(const std::string & name, const std::string & argumentName,
            void (*callback)(void*), TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace);

        static void registerMethodWithoutArg(const std::string & name, void (*callback)(void*));

        void init();

    private:
        DebugConsole() {}
        ~DebugConsole() {}

    private:
        std::map<std::string, void (*)(void*)> name2callback;

        TwBar* bar;
};

#endif
