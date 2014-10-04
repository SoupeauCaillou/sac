#include "base/Log.h"

#define _GAME_CLASS(n) n##Game
#define GAME_CLASS(n) _GAME_CLASS(n)

#define _GAME_HEADER(n) n##Game.h
#define GAME_HEADER(n) _GAME_HEADER(n)

#define _GIT_VERSION_HEADER(n) n##GitVersion.h
#define GIT_VERSION_HEADER(n) _GIT_VERSION_HEADER(n)

#define XSTR(s) STR(s)
#define STR(s) #s

#define INCLUDE_NAME() XSTR(GAME_HEADER(PROJECT_NAME))
#include INCLUDE_NAME()

#if SAC_DESKTOP || SAC_MOBILE
Game* buildGameInstance() {
    return new GAME_CLASS(PROJECT_NAME) ();
}

#endif

#if SAC_DESKTOP
#define INCLUDE_VERSION() XSTR(GIT_VERSION_HEADER(PROJECT_NAME))
#include INCLUDE_VERSION()
#include "app/AppSetup.h"

/* Entry Point */
int main(int argc, char** argv) {
    std::string versionName = "";
    #if SAC_DEBUG
        versionName = "";//versionName + " / " + TAG_NAME + " - " + VERSION_NAME;
    #endif

    if (initGame(XSTR(PROJECT_NAME), versionName)) {
        LOGE("Failed to initialize");
        return 1;
    }

    return launchGame(
        new GAME_CLASS(PROJECT_NAME) (),
        argc,
        argv);
}
#endif

