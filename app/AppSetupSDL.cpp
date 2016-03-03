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

#include "app/AppSetup.h"
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

#if SAC_LINUX
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
#include "api/linux/KeyboardInputHandlerAPISDLImpl.h"
#include "api/linux/MusicAPILinuxOpenALImpl.h"
#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/linux/OpenURLAPILinuxImpl.h"
#include "api/linux/SoundAPILinuxOpenALImpl.h"
#include "api/linux/StringInputAPISDLImpl.h"
#include "api/linux/VibrateAPILinuxImpl.h"
#include "api/linux/WWWAPIcURLImpl.h"
#include "api/default/LocalizeAPITextImpl.h"
#include "api/default/GameCenterAPIDebugImpl.h"
#include "api/default/SqliteStorageAPIImpl.h"
#include "api/default/AdAPIDebugImpl.h"
#include "api/default/InAppPurchaseAPIDebugImpl.h"

#include "util/Recorder.h"
#include "util/Draw.h"

#include "api/sdl/JoystickAPISDLImpl.h"
#include "api/sdl/MouseNativeTouchState.h"

#include "util/LevelEditor.h"

Game* game = 0;
SDL_Window* sdlWindow = 0;
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
static std::condition_variable cond;

static void updateLoop(const std::string& ) {
    while(! game->isFinished) { // && (SDL_GetAppState() & SDL_APPACTIVE)) {

#if SAC_BENCHMARK_MODE
    updateBench();
#endif
        game->step();

        bool focus = (SDL_GetKeyboardFocus() == sdlWindow);
#if !SAC_DEBUG
        //enable music only if we have the focus
        theMusicSystem.toggleMute(!focus);
#endif
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

static void addWindowIcon(SDL_Window* window);
static std::string getLocaleInfo();

// hum hum
extern bool profilerEnabled;

struct CommandLineOptions {
    CommandLineOptions() {
        restore = false;
        verbose = 0;
        forceEtc1 = false;
        headless = false;
        profiler = false;
#if SAC_NETWORK
        nickname = NULL;
        lobby = NULL;
#endif
    }
    bool restore;
    int verbose;
    bool forceEtc1;
    bool headless;
    bool profiler;
#if SAC_NETWORK
    const char* nickname;
    const char* lobby;
#endif
};
static CommandLineOptions parseCommandLineOption(int argc, char** argv);

int setupEngine(void* _game, const SetupInfo* info) {
    #if SAC_DESKTOP && SAC_LINUX && SAC_ENABLE_LOG
        initLogColors();
    #endif

    /////////////////////////////////////////////////////
    // Init Window and Rendering
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return 1;
    }

    if ((sdlWindow = SDL_CreateWindow(info->name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE )) == 0) {
        LOGE("SDL create window failed: " << SDL_GetError());
        return 1;
    }

    addWindowIcon(sdlWindow);

    game = static_cast<Game*> (_game);

#if SAC_DESKTOP
    game->arg.c = info->arg.c;
    game->arg.v = new char*[info->arg.c];
    for (int i=0; i<info->arg.c; i++) {
        game->arg.v[i] = strdup(info->arg.v[i]);
    }
#endif

    /////////////////////////////////////////////////////
    // Handle --restore cmd line switch
    uint8_t* state = 0;
    int size = 0;

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
    CommandLineOptions options;
#else
    auto options =
        parseCommandLineOption(info->arg.c, info->arg.v);
#endif


    int largestDimension = 1024;
    float w2hRatio = 10 / 16.0f;
    if (!game->isLandscape()) {
        w2hRatio = 1.0f / w2hRatio;
    }

    {
        SDL_DisplayMode mode;
        if (0 == SDL_GetCurrentDisplayMode(0, &mode)) {
            while (mode.w < largestDimension ||
                mode.h < (largestDimension * w2hRatio)) {
                largestDimension *= 0.8;
            }
        }
    }
    glm::vec2 resolution (largestDimension, largestDimension * w2hRatio);

    SDL_SetWindowSize(sdlWindow, resolution.x, resolution.y);
    SDL_GLContext sdlContext;
    if  ((sdlContext = SDL_GL_CreateContext(sdlWindow)) == 0) {
        LOGE("SDL create context failed: " << SDL_GetError());
        return 1;
    }

#if !SAC_EMSCRIPTEN
    if (glewInit() != GLEW_OK)
        return 1;

    if (options.restore) {
        LOGW("FIXME: probably broken");
        std::stringstream restoreFile;
        restoreFile << info->name << ".restore.bin";
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

    #if SAC_ENABLE_LOG
        if (options.verbose) {
            logLevel = LogVerbosity::VERBOSE2;
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
    if (game->wantsAPI(ContextAPI::Joystick))
        ctx->joystickAPI = new JoystickAPISDLImpl();
#if !SAC_INGAME_EDITORS
    if (game->wantsAPI(ContextAPI::KeyboardInputHandler))
#endif
        ctx->keyboardInputHandlerAPI = new KeyboardInputHandlerAPISDLImpl();
    if (game->wantsAPI(ContextAPI::Localize))
        ctx->localizeAPI = new LocalizeAPITextImpl();
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
        static_cast<AssetAPILinuxImpl*>(ctx->assetAPI)->init(info->name);
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
        static_cast<LocalizeAPITextImpl*>(ctx->localizeAPI)->init(ctx->assetAPI, getLocaleInfo().c_str());
    }

    /////////////////////////////////////////////////////
    // Init game
    LOGV(1, "Initialize sac & game");
    game->setGameContexts(ctx, ctx);
    sac::setResolution(resolution.x, resolution.y);
    game->sacInitFromRenderThread();

#if SAC_DESKTOP
    if (options.forceEtc1) {
        OpenGLTextureCreator::forceEtc1Usage();
    }
#endif

    game->sacInitFromGameThread();
    game->init(state, size);

    #if SAC_NETWORK
    {
        NetworkAPILinuxImpl* api = static_cast<NetworkAPILinuxImpl*> (theNetworkSystem.networkAPI);
        api->init();
    }
    #endif

    if (!options.headless)
        theRenderingSystem.enableRendering();

    LOGV(1, "Run game loop");

    #if SAC_EMSCRIPTEN
        emscripten_set_main_loop(updateAndRender, 0, 0);
        return 0;
    #else

    #if SAC_ENABLE_PROFILING
        if (options.profiler)
            startProfiler();
    #endif

    SDL_JoystickEventState(SDL_ENABLE);

    //used for text translation, if needed
    setlocale( LC_ALL, "" );
    setlocale( LC_NUMERIC, "C" );

    std::unique_lock<std::mutex> lock(m);
    std::thread th1(callback_thread, info->name);
    cond.wait(lock);
    lock.unlock();

    float prevT = 0;

    do {
        game->eventsHandler();
        if (!options.headless) {
            game->render();
            SDL_GL_SwapWindow(sdlWindow);
            float t = TimeUtil::GetTime();
#if ! SAC_WINDOWS
            Recorder::Instance().record(t - prevT);
#endif
            prevT = t;
        }
    } while (!game->isFinished); //!m.try_lock());

    th1.join();

    LOGT("We should destroy API to let them uninit stuff "
        "(JoystickManager, MusicAPILinuxOpenALImplOpenAL, ...) + fix memory leaks");
    delete ctx;

#if SAC_DESKTOP
    for (int i=0; i<game->arg.c; i++) {
        free(game->arg.v[i]);
    }
    delete[] game->arg.v;
#endif
#endif
    game->preDestroy();
    delete game;
 //   delete record;
    SDL_GL_DeleteContext(sdlContext);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    return 0;
}

//return user locale (DE, EN, FR, etc.)
static std::string getLocaleInfo() {
    #if SAC_WINDOWS
        WCHAR szISOLang[5] = {0};
        WCHAR szISOCountry[5] = {0};

        ::GetLocaleInfo(LOCALE_USER_DEFAULT,
                        LOCALE_SISO639LANGNAME,
                        (LPSTR)szISOLang,
                        sizeof(szISOLang) / sizeof(WCHAR));

        ::GetLocaleInfo(LOCALE_USER_DEFAULT,
                        LOCALE_SISO3166CTRYNAME,
                        (LPSTR)szISOCountry,
                        sizeof(szISOCountry) / sizeof(WCHAR));

        std::wstring ws(szISOCountry);

        std::string lang((const char*)&ws[0], sizeof(wchar_t)/sizeof(char)*ws.size());
    #elif SAC_EMSCRIPTEN
        std::string lang = emscripten_run_script_string( "navigator.language;" );
        lang.resize(2);
    #elif SAC_IOS
        LOGT("[[NSLocale preferredLangagues] objectAtIndex:0]");
        std::string lang = "en";
    #elif SAC_ANDROID
        LOGT("Findout user locale");
        std::string lang = "en";
    #else
        std::string lang(getenv("LANG"));
        //cut part after the '_' underscore
        lang.resize(2);
    #endif

    //convert to lower case
    transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

    LOGV(1, "Using locale: '" << lang << "'");
    return lang;
}

static void addWindowIcon(SDL_Window* window) {
    #if ! SAC_EMSCRIPTEN
    std::stringstream iconPath;
    AssetAPILinuxImpl api;

    // first check if android logo available (the default one)
    iconPath << SAC_ASSETS_DIR << "../android/res/drawable-hdpi/icon.png";
    // if the game is not android compliant, then look for an icon.png asset
    if (! api.doesExistFileOrDirectory(iconPath.str())) {
        iconPath.str("");
        iconPath << SAC_ASSETS_DIR << "icon.png";
    }
    if (api.doesExistFileOrDirectory(iconPath.str())) {
        FileBuffer fb;
        ImageDesc image = ImageLoader::loadPng(iconPath.str(),fb);
        SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(image.datas,
            image.width, image.height,
            image.channels * 8,
            image.width * image.channels,
            0xFF << 00, 0xFF << 8, 0xFF << 16, 0xFF << 24
            );

        SDL_SetWindowIcon(window, surf);
        SDL_FreeSurface(surf);
        free(fb.data);
        free(image.datas);
    }
    #endif
}

static CommandLineOptions parseCommandLineOption(int argc, char** argv) {
    CommandLineOptions options;

    for (int i=1; i<argc; i++) {
        options.restore |= !strcmp(argv[i], "-restore");
        options.verbose |= !strcmp(argv[i], "-v");
        options.verbose |= !strcmp(argv[i], "--verbose");
        options.headless |= !strcmp(argv[i], "--headless");
        options.forceEtc1 |= !strcmp(argv[i], "--force-etc1");
        options.profiler |= !strcmp("-profile", argv[i]);
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
    #if SAC_NETWORK
        if (!strcmp(argv[i], "--nickname")) {
            options.nickname = argv[++i];
        }
        if (!strcmp(argv[i], "--lobby")) {
            options.lobby = argv[++i];
        }
    #endif
    }
    return options;
}

