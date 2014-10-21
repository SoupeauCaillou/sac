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
#include "base/Common.h"

#include <SDL.h>

#if SAC_DESKTOP
#include "systems/opengl/OpenGLTextureCreator.h"

#endif
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
#include "util/Draw.h"

#include "MouseNativeTouchState.h"

#include "util/LevelEditor.h"

Game* game = 0;

#if SAC_BENCHMARK_MODE
static int frameCount = 0;
static uint64_t totalFrameCount = 0;
static float startTime = 5;

static void updateBench() {
    float t = TimeUtil::GetTime();


    float dt = t - startTime;
    if (dt > 0) {
        frameCount++;
        if (dt >= 1) {
            std::cout << frameCount << " frames in " << dt << " sec. Avg: " << (1000 * dt) /frameCount << " ms/frame\n";
            totalFrameCount += frameCount;
            frameCount = 0;
            startTime = t;
        }
    }
    /*
    if (t > 20) {
        std::cout << "TOTAL FRAME COUNT:" << totalFrameCount << std::endl;
        exit(0);
    }
    */
}
#endif

#if SAC_EMSCRIPTEN
static void updateAndRender() {
#if !SAC_BENCHMARK_MODE
    LOGV(1, "gameloop - events handle");
    game->eventsHandler();
#else
    updateBench();
#endif
    LOGV(1, "gameloop - step");
    game->step();
    LOGV(1, "gameloop - render");
#if !SAC_BENCHMARK_MODE
    game->render();
#endif
}
#else
static std::mutex m;
static bool updateThreadReady;
static std::condition_variable cond;

static void updateLoop(const std::string& ) {
    while(! game->isFinished) { // && (SDL_GetAppState() & SDL_APPACTIVE)) {

#if SAC_BENCHMARK_MODE
    updateBench();
#endif
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

static void* callback_thread(const std::string& gameName){
    {
        std::unique_lock<std::mutex> lock(m);
        cond.notify_all();
    }

    updateLoop(gameName);

    return NULL;
}
#endif

// hum hum
extern bool profilerEnabled;
std::string gameName;
int initGame(const std::string& gameN, const std::string& gameVersion) {
#if SAC_DESKTOP && SAC_LINUX && SAC_ENABLE_LOG
    initLogColors();
#endif

    gameName = gameN;

    /////////////////////////////////////////////////////
    // Init Window and Rendering
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        return 1;
    }

#if !SAC_EMSCRIPTEN
    // hard coded icon path
    {
        std::stringstream iconPath;
        iconPath << SAC_ASSETS_DIR << "../android/res/drawable-hdpi/ic_launcher.png";
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
#endif

    //set title - display current revision too (debug purpose)
    SDL_WM_SetCaption((gameName + gameVersion).c_str(), 0);

    SDL_EnableUNICODE(1);

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
    bool restore = false, verbose = false, forceEtc1 = false, headless = false;
#if SAC_ENABLE_PROFILING
    profilerEnabled = false;
#endif
    for (int i=1; i<argc; i++) {
        restore |= !strcmp(argv[i], "-restore");
        verbose |= !strcmp(argv[i], "-v");
        verbose |= !strcmp(argv[i], "--verbose");
        headless |= !strcmp(argv[i], "--headless");
        forceEtc1 |= !strcmp(argv[i], "--force-etc1");
#if SAC_INGAME_EDITORS
        if (!strcmp(argv[i], "--debug-area-width") ||
            !strcmp(argv[i], "-d-a-w")) {
            LOGF_IF((i+1)>= argc, "Invalid argument count. Expecting integer");
            LevelEditor::DebugAreaWidth = std::atoi(argv[i+1]);
            i++;
        } else
        if (!strcmp(argv[i], "--debug-area-height") ||
            !strcmp(argv[i], "-d-a-h")) {
            LOGF_IF((i+1)>= argc, "Invalid argument count. Expecting integer");
            LevelEditor::DebugAreaHeight = std::atoi(argv[i+1]);
            i++;
        }

#endif
#if SAC_ENABLE_PROFILING
        profilerEnabled |= !strcmp("-profile", argv[i]);
#endif
    }


    // Double Buffering
    // Warning! This method DOES call srand (random generator)
    glm::vec2 resolution(0, 600);
    if (game->isLandscape()) {
        resolution.x = 800;
    } else {
        resolution.x = 375;
    }

#if SAC_INGAME_EDITORS
    if (SDL_SetVideoMode(resolution.x + LevelEditor::DebugAreaWidth, resolution.y + LevelEditor::DebugAreaHeight, 32, SDL_OPENGL | SDL_RESIZABLE) == 0)
#else
    if (SDL_SetVideoMode(resolution.x, resolution.y, 32, SDL_OPENGL | SDL_RESIZABLE ) == 0)
#endif
        return 1;

#if ! SAC_EMSCRIPTEN
    if (glewInit() != GLEW_OK)
        return 1;
#endif

    if (restore) {
        // TODO: portability
        std::stringstream restoreFile;
        restoreFile << gameName << ".restore.bin";
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

#if SAC_ENABLE_LOG
    if (verbose) {
        logLevel = LogVerbosity::VERBOSE1;
    }
#endif
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
#if !SAC_INGAME_EDITORS
    if (game->wantsAPI(ContextAPI::KeyboardInputHandler))
#endif
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
#if SAC_HAVE_CURL
    if (game->wantsAPI(ContextAPI::WWW))
        ctx->wwwAPI = new WWWAPIcURLImpl();
#endif
    /////////////////////////////////////////////////////
    // Init systems
    game->mouseNativeTouchState = new MouseNativeTouchState();
    theTouchInputManager.setNativeTouchStatePtr(game->mouseNativeTouchState);
    theRenderingSystem.assetAPI = ctx->assetAPI;

    if (game->wantsAPI(ContextAPI::Asset) || true) {
        static_cast<AssetAPILinuxImpl*>(ctx->assetAPI)->init(gameName);
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
    sac::setResolution(resolution.x, resolution.y);
    game->sacInit();

#if SAC_DESKTOP
    if (forceEtc1) {
        OpenGLTextureCreator::forceEtc1Usage();
    }
#endif
    game->init(state, size);

    if (!headless)
        theRenderingSystem.enableRendering();

    LOGV(1, "Run game loop");

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

#if SAC_EMSCRIPTEN
    emscripten_set_main_loop(updateAndRender, 0, 0);
#else

#if SAC_ENABLE_PROFILING
    if (profilerEnabled)
        startProfiler();
#endif

    SDL_JoystickEventState(SDL_ENABLE);

    //used for text translation, if needed
    setlocale( LC_ALL, "" );
    setlocale( LC_NUMERIC, "C" );

    std::unique_lock<std::mutex> lock(m);
    std::thread th1(callback_thread, gameName);
    cond.wait(lock);
    lock.unlock();

    float prevT = 0;

    do {
        game->eventsHandler();
        if (!headless) {
            game->render();
            SDL_GL_SwapBuffers();
            float t = TimeUtil::GetTime();
#if ! SAC_WINDOWS
            Recorder::Instance().record(t - prevT);
#endif
            prevT = t;
        }
    } while (!game->isFinished); //!m.try_lock());

    th1.join();

    delete ctx;

    Draw::ClearAll();

    /* Delete before destroying game, as system will be destroyed */
    theEntityManager.deleteAllEntities();

    delete game;
 //   delete record;
    SDL_Quit();

#if SAC_INGAME_EDITORS

#endif

#endif


    return 0;
}

