/*
	This file is part of sac.

	@author Soupe au Caillou

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "base/Game.h"
#include "base/GameContext.h"

#include <SDL/SDL.h>

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <vector>
#include <signal.h>
#include <thread>
#include <mutex>

#if SAC_LINUX || SAC_ANDROID
#include <locale.h>
#include <libintl.h>
#endif

#include <glm/glm.hpp>
#include "base/TouchInputManager.h"
#include "base/TimeUtil.h"
#include "base/PlacementHelper.h"
#include "base/Profiler.h"
#include "systems/RenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "api/AdAPI.h"
#include "api/linux/AssetAPILinuxImpl.h"
#include "api/linux/CommunicationAPILinuxImpl.h"
#include "api/linux/ExitAPILinuxImpl.h"
#include "api/linux/LocalizeAPILinuxImpl.h"
#include "api/linux/MusicAPILinuxOpenALImpl.h"
#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/linux/SoundAPILinuxOpenALImpl.h"
#include "api/linux/VibrateAPILinuxImpl.h"
#include "api/default/KeyboardInputHandlerAPIGLFWImpl.h"
#include "api/default/SqliteStorageAPIImpl.h"
#include "api/SuccessAPI.h"

#include "util/Recorder.h"

#include "MouseNativeTouchState.h"

Game* game = 0;
Recorder *record; //only on linux

#if SAC_EMSCRIPTEN
static void updateAndRender() {
    LOGV(1, "gameloop - events handle");
    game->eventsHandler();
    LOGV(1, "gameloop - step");
    game->step();
    LOGV(1, "gameloop - render");
    game->render();
}
#else
std::mutex m;

static void updateLoop() {
    while(! game->isFinished && (SDL_GetAppState() & SDL_APPACTIVE)) {
        // game->eventsHandler();

        game->step();

        bool focus = (SDL_GetAppState() & SDL_APPINPUTFOCUS);
        if (focus) {
            theMusicSystem.toggleMute(theSoundSystem.mute);
        } else {
            // theMusicSystem.toggleMute(true);
        }
        //pause ?

        // recording
        if (SDL_GetKeyState(NULL)[SDLK_F10]) {
            record->stop();
        }
        if (SDL_GetKeyState(NULL)[SDLK_F9]) {
            record->start();
        }
    }
    theRenderingSystem.disableRendering();
}

static void* callback_thread(){
    m.lock();
    updateLoop();
    m.unlock();
    return NULL;
}
#endif

glm::vec2 resolution;
int initGame(const std::string& title, const glm::ivec2& res) {
    resolution = res;

    /////////////////////////////////////////////////////
    // Init Window and Rendering
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return 1;
    }
    SDL_WM_SetCaption(title.c_str(), 0);

    // Double Buffering
    if (SDL_SetVideoMode(resolution.x, resolution.y, 32, SDL_OPENGL ) == 0)
        return 1;

#if ! SAC_EMSCRIPTEN
    if (glewInit() != GLEW_OK)
        return 1;
#endif

    return 0;
}

/** Engine entry point */
int launchGame(Game* gameImpl, int argc, char** argv) {
    game = gameImpl;

    /////////////////////////////////////////////////////
    // Handle --restore cmd line switch
    uint8_t* state = 0;
    int size = 0;

    //emscripten doesn't handle restore functionnality
#if ! SAC_EMSCRIPTEN
    bool restore = false;
    for (int i=1; i<argc; i++) {
        restore |= !strcmp(argv[i], "-restore");
    }

    if (restore) {
        // TODO: portability
        std::stringstream restoreFile;
        restoreFile << "/tmp/" << "todo" << ".bin";
        FILE* file = fopen(restoreFile.str().c_str(), "r+b");
        if (file) {
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 0, SEEK_SET);
            state = new uint8_t[size];
            fread(state, size, 1, file);
            fclose(file);
            LOGI("Restoring game state from file (size: " << size << ")");
        }
    }
#endif

    /////////////////////////////////////////////////////
    // Game context initialisation
    LOGV(1, "Initialize APIs");
    GameContext* ctx = new GameContext;
    if (game->wantsAPI(ContextAPI::Ad))
        ctx->adAPI = new AdAPI();
    if (game->wantsAPI(ContextAPI::Asset) || true)
        ctx->assetAPI = new AssetAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Communication))
        ctx->communicationAPI = new CommunicationAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Exit))
        ctx->exitAPI = new ExitAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::KeyboardInputHandler))
        ctx->keyboardInputHandlerAPI = new KeyboardInputHandlerAPIGLFWImpl();
    if (game->wantsAPI(ContextAPI::Localize))
        ctx->localizeAPI = new LocalizeAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Music))
        ctx->musicAPI = new MusicAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::Network))
        ctx->networkAPI = new NetworkAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Sound))
        ctx->soundAPI = new SoundAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::Storage))
        ctx->storageAPI = new SqliteStorageAPIImpl();
    if (game->wantsAPI(ContextAPI::Success))
        ctx->successAPI = new SuccessAPI();
    if (game->wantsAPI(ContextAPI::Vibrate))
        ctx->vibrateAPI = new VibrateAPILinuxImpl();

    /////////////////////////////////////////////////////
    // Init systems
    game->mouseNativeTouchState = new MouseNativeTouchState();
    theTouchInputManager.setNativeTouchStatePtr(game->mouseNativeTouchState);
    theRenderingSystem.assetAPI = ctx->assetAPI;
    if (game->wantsAPI(ContextAPI::Music)) {
        theMusicSystem.musicAPI = ctx->musicAPI;
        theMusicSystem.assetAPI = ctx->assetAPI;
        static_cast<MusicAPILinuxOpenALImpl*>(ctx->musicAPI)->init();
        theMusicSystem.init();
    }
    if (game->wantsAPI(ContextAPI::Sound)) {
        theSoundSystem.soundAPI = ctx->soundAPI;
        static_cast<SoundAPILinuxOpenALImpl*>(ctx->soundAPI)->init(ctx->assetAPI, game->wantsAPI(ContextAPI::Music));
        theSoundSystem.init();
    }
    if (game->wantsAPI(ContextAPI::Localize)) {
        char* lang = strdup(getenv("LANG"));
        lang[2] = '\0';
        static_cast<LocalizeAPILinuxImpl*>(ctx->localizeAPI)->init(ctx->assetAPI, lang);
    }

    /////////////////////////////////////////////////////
    // Init game
    LOGV(1, "Initialize sac & game");
    game->setGameContexts(ctx, ctx);
    game->sacInit(resolution.x, resolution.y);
    game->init(state, size);

    record = new Recorder(resolution.x, resolution.y);

    LOGV(1, "Run game loop");
#if SAC_EMSCRIPTEN
    emscripten_set_main_loop(updateAndRender, 60, 0);
#else

    //used for text translation, if needed
    setlocale( LC_ALL, "" );

    std::thread th1(callback_thread);
    do {
        game->eventsHandler();
        game->render();
        SDL_GL_SwapBuffers();
        record->record();
    } while (!m.try_lock());
    th1.join();

    delete ctx;
    delete game;
 //   delete record;
    SDL_Quit();
#endif
    return 0;
}

