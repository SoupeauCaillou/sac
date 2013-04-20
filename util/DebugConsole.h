#if SAC_INGAME_EDITORS

#include <string>
#include <map>

class DebugConsole {
    public:
        static DebugConsole & Instance();

        void registerMethod(const std::string & name, void (*callback)(std::string*));

        void invoke(const std::string & name, std::string* args);
    private:
        DebugConsole();
        ~DebugConsole();

    private:
        std::map<std::string, void (*)(std::string*)> name2callback;
};

#endif
