#if SAC_DESKTOP

#include "../../sources/GitVersion.h"
#include "app/AppSetup.h"

#define _GAME_CLASS(n) n##Game
#define GAME_CLASS(n) _GAME_CLASS(n)

#define _GAME_HEADER(n) n##Game.h
#define GAME_HEADER(n) _GAME_HEADER(n)

 #define XSTR(s) STR(s)
#define STR(s) #s

#define INCLUDE_NAME() XSTR(GAME_HEADER(PROJECT_NAME))
#include INCLUDE_NAME()


/* Entry Point */
int main(int argc, char** argv) {
    std::string versionName = "";
    #if SAC_DEBUG
        versionName = "TODO";//versionName + " / " + TAG_NAME + " - " + VERSION_NAME;
    #endif

    if (initGame(STR(PROJECT_NAME), glm::ivec2(800, 600), versionName)) {
        LOGE("Failed to initialize");
        return 1;
    }

    return launchGame(
        new GAME_CLASS(PROJECT_NAME) (),
        argc,
        argv);
}
#endif
