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



#include "base/Game.h"
#include "base/GameContext.h"

#include <SDL.h>

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
#include "systems/TextSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/NetworkSystem.h"

#include "api/AdAPI.h"
#include "api/linux/AssetAPILinuxImpl.h"
#include "api/linux/CommunicationAPILinuxImpl.h"
#include "api/linux/ExitAPILinuxImpl.h"
#include "api/linux/GameCenterAPIDebugImpl.h"
#include "api/linux/KeyboardInputHandlerAPISDLImpl.h"
#include "api/linux/LocalizeAPILinuxImpl.h"
#include "api/linux/MusicAPILinuxOpenALImpl.h"
#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/linux/OpenURLAPILinuxImpl.h"
#include "api/linux/SoundAPILinuxOpenALImpl.h"
#include "api/linux/StringInputAPISDLImpl.h"
#include "api/linux/VibrateAPILinuxImpl.h"
#include "api/linux/WWWAPIcURLImpl.h"
#include "api/default/SqliteStorageAPIImpl.h"
#include "api/default/AdAPIDebugImpl.h"
#include "api/default/InAppPurchaseAPIDebugImpl.h"

#include "util/Recorder.h"

#include "MouseNativeTouchState.h"

Game* game = 0;

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

static void updateLoop(const std::string& title) {
    unsigned char * keys = SDL_GetKeyState(NULL);

    while(! game->isFinished && (SDL_GetAppState() & SDL_APPACTIVE)) {
#if SAC_DESKTOP
        if (keys[SDLK_LSHIFT]) {
            LOGI("****************** Testing Restore");
            uint8_t* ptr;
            int size = game->saveState(&ptr);

            if (size == 0) {
                LOGI("*  Nothing saved by game... continuing execution then");
            } else {
                std::stringstream name;
                name << title << ".restore.bin";
                LOGI("*  " << size << " bytes saved. Writing to '" << name.str() << "' file");
                std::ofstream of(name.str(), std::ios::binary);
                of.write((char*)ptr, size);
                of.close();
                LOGI("*  Now exiting");
                break;
            }
        }
#endif
        // game->eventsHandler();

        game->step();

        bool focus = (SDL_GetAppState() & SDL_APPINPUTFOCUS);
        //if we lost the focus, mute the music (release mode only)
        if (! focus) {
            #if ! SAC_DEBUG
                theMusicSystem.toggleMute(true);
            #endif
        //otherwise, restore sound as it was before focus was lost
        } else {
            theMusicSystem.toggleMute(theSoundSystem.mute);
        }
    }
    theRenderingSystem.disableRendering();
}

static void* callback_thread(const std::string& title){
    m.lock();
    updateLoop(title);
    m.unlock();
    return NULL;
}
#endif

// hum hum
extern bool profilerEnabled;
glm::vec2 resolution;
std::string title;
int initGame(const std::string& pTitle, const glm::ivec2& res) {
    resolution = res;
    title = pTitle;

    /////////////////////////////////////////////////////
    // Init Window and Rendering
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        return 1;
    }

    // hard coded icon path
    {
        std::stringstream iconPath;
        iconPath << SAC_ASSETS_DIR << "../res/drawable-hdpi/ic_launcher.png";
        AssetAPILinuxImpl api;
        FileBuffer fb = api.loadFile(iconPath.str());
        if (fb.size) {
            ImageDesc image = ImageLoader::loadPng(iconPath.str(), fb);
            SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(image.datas,
                image.width, image.height,
                image.channels * 8,
                image.width * image.channels,
                0xFF << 00, 0xFF << 8, 0xFF << 16, 0xFF << 24
                );

            SDL_WM_SetIcon(surf, NULL);
            SDL_FreeSurface(surf);
            delete[] fb.data;
            delete[] image.datas;
        }
    }

    //display git revision if available
    #ifdef SAC_REVISION_TAG
        SDL_WM_SetCaption((title + " " + SAC_REVISION_TAG).c_str(), 0);
    #else
        SDL_WM_SetCaption(title.c_str(), 0);
    #endif

    SDL_EnableUNICODE(1);

    // Double Buffering
    // Warning! This method DOES call srand (random generator)
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
#if SAC_EMSCRIPTEN
    const char* script = ""\
        "var r = localStorage.getItem(\"sac_root\");" \
        "if (r != null) { Module.print('Restoring root');"\
        "   FS.root.contents['sac_temp'] = window.JSON.parse(r); Module.print('Restoring nextItem');"\
        "   FS.nextInode = window.JSON.parse(localStorage.getItem(\"sac_nextItem\"));"\
        " } else { "\
        "Module['FS_createFolder']('/', 'sac_temp', true, true);" \
        "}";
    emscripten_run_script(script);

#else
    bool restore = false;
#if SAC_ENABLE_PROFILING
    profilerEnabled = false;
#endif
    for (int i=1; i<argc; i++) {
        restore |= !strcmp(argv[i], "-restore");
#if SAC_ENABLE_PROFILING
        profilerEnabled |= !strcmp("-profile", argv[i]);
#endif
    }

    if (restore) {
        // TODO: portability
        std::stringstream restoreFile;
        restoreFile << title << ".restore.bin";
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
        ctx->adAPI = new AdAPIDebugImpl();
    if (game->wantsAPI(ContextAPI::Asset) || true)
        ctx->assetAPI = new AssetAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Communication))
        ctx->communicationAPI = new CommunicationAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Exit))
        ctx->exitAPI = new ExitAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::GameCenter))
        ctx->gameCenterAPI = new GameCenterAPIDebugImpl();
    if (game->wantsAPI(ContextAPI::InAppPurchase))
        ctx->inAppPurchaseAPI = new InAppPurchaseAPIDebugImpl();
    if (game->wantsAPI(ContextAPI::KeyboardInputHandler))
        ctx->keyboardInputHandlerAPI = new KeyboardInputHandlerAPIGLFWImpl();
    if (game->wantsAPI(ContextAPI::Localize))
        ctx->localizeAPI = new LocalizeAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Music))
        ctx->musicAPI = new MusicAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::OpenURL))
        ctx->openURLAPI = new OpenURLAPILinuxImpl();
#if SAC_NETWORK
    if (game->wantsAPI(ContextAPI::Network))
        theNetworkSystem.networkAPI = ctx->networkAPI = new NetworkAPILinuxImpl();
#elif SAC_DESKTOP
    if (game->wantsAPI(ContextAPI::Network))
        LOGF("You wanted NetworkAPI but did not used SAC_NETWORK!");
#endif
    if (game->wantsAPI(ContextAPI::Sound))
        ctx->soundAPI = new SoundAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::Storage))
       ctx->storageAPI = new SqliteStorageAPIImpl();
    if (game->wantsAPI(ContextAPI::StringInput))
        ctx->stringInputAPI = new StringInputAPISDLImpl();
    if (game->wantsAPI(ContextAPI::Vibrate))
        ctx->vibrateAPI = new VibrateAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::WWW))
        ctx->wwwAPI = new WWWAPIcURLImpl();
    /////////////////////////////////////////////////////
    // Init systems
    game->mouseNativeTouchState = new MouseNativeTouchState();
    theTouchInputManager.setNativeTouchStatePtr(game->mouseNativeTouchState);
    theRenderingSystem.assetAPI = ctx->assetAPI;

    if (game->wantsAPI(ContextAPI::Asset) || true) {
        static_cast<AssetAPILinuxImpl*>(ctx->assetAPI)->init(title);
    }
	
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
        static_cast<LocalizeAPILinuxImpl*>(ctx->localizeAPI)->init(ctx->assetAPI);
    }

    /////////////////////////////////////////////////////
    // Init game
    LOGV(1, "Initialize sac & game");
    game->setGameContexts(ctx, ctx);
    game->sacInit(resolution.x, resolution.y);
    game->init(state, size);

    theRenderingSystem.enableRendering();

#if SAC_LINUX && SAC_DESKTOP
    Recorder::Instance().init(resolution.x, resolution.y);
#endif
    
    LOGV(1, "Run game loop");

    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

#if SAC_EMSCRIPTEN
    emscripten_set_main_loop(updateAndRender, 60, 0);
#else

#if SAC_ENABLE_PROFILING
    if (profilerEnabled)
        startProfiler();
#endif



    //used for text translation, if needed
    setlocale( LC_ALL, "" );
    setlocale( LC_NUMERIC, "C" );

    std::thread th1(callback_thread, title);
    float prevT = 0;
#if SAC_DEBUG
    std::string currentHint = game->titleHint;
#endif

    do {
        game->eventsHandler();
        game->render();
        SDL_GL_SwapBuffers();
        float t = TimeUtil::GetTime();
#if ! SAC_WINDOWS
        Recorder::Instance().record(t - prevT);
#endif
        prevT = t;

#if SAC_DEBUG
        if (game->titleHint != currentHint) {
            std::stringstream str;
            str << title;
#ifdef SAC_REVISION_TAG
            str << " / " << SAC_REVISION_TAG;
#endif
            str << " / " << (currentHint = game->titleHint);
            SDL_WM_SetCaption(str.str().c_str(), 0);
        }
#endif
    } while (!m.try_lock());
    th1.join();

    delete ctx;

    theEntityManager.deleteAllEntities();

    delete game;
 //   delete record;
    SDL_Quit();

#if SAC_INGAME_EDITORS
    TwTerminate();
#endif

#endif


    return 0;
}

