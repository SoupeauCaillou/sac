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
#ifdef __cplusplus
extern "C" {
#endif
Game* buildGameInstance() {
    return new GAME_CLASS(PROJECT_NAME) ();
}
#ifdef __cplusplus
}
#endif
#endif

#if SAC_DESKTOP
#define INCLUDE_VERSION() XSTR(GIT_VERSION_HEADER(PROJECT_NAME))
#include INCLUDE_VERSION()
#include "app/AppSetup.h"

/* Entry Point */
int main(int argc, char** argv) {

    SetupInfo info;
    info.name = STR(PROJECT_NAME);
    info.arg.c = argc;
    info.arg.v = argv;

    auto* game = buildGameInstance();
    setupEngine(game, &info);
}
#endif

