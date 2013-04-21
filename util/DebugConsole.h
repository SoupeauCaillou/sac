#if SAC_INGAME_EDITORS

#include "base/Log.h"

#include <string>
#include <map>

#include <AntTweakBar.h>


//use this method to add a method to the debug console
#define REGISTER(name, args)\
    static int name##Value = args[0].Value;\
    DebugConsole::Instance().registerMethod(#name, #args, &callback##name, args, sizeof(args)/sizeof(TwEnumVal), &name##Value);


class DebugConsole {
    public:
        static DebugConsole & Instance();

        static void registerMethod(const std::string & name, const std::string & argumentName,
            void (*callback)(void*), TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace);

        void init();

    private:
        DebugConsole() {}
        ~DebugConsole() {}

    private:
        std::map<std::string, void (*)(void*)> name2callback;

        TwBar* bar;
};

#endif
