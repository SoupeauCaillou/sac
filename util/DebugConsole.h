#if SAC_INGAME_EDITORS

#include <string>
#include <map>
#include <list>

#include <AntTweakBar.h>

class DebugConsole {
    public:
        static DebugConsole & Instance();

        void registerMethod(const std::string & name, void (*callback)(void*));

        void setBar(TwBar* debugConsoleBar) { bar = debugConsoleBar; }

    private:
        DebugConsole() {}
        ~DebugConsole() {}

    private:
        std::map<std::string, void (*)(void*)> name2callback;

        TwBar* bar;
};

#endif
