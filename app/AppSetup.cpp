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

#if SAC_EMSCRIPTEN
#include <SDL/SDL.h>
#include <emscripten/emscripten.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>
#endif

#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <assert.h>
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
#include "api/linux/NameInputAPILinuxImpl.h"
#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/linux/SoundAPILinuxOpenALImpl.h"
#include "api/linux/StorageAPILinuxImpl.h"
#include "api/linux/VibrateAPILinuxImpl.h"
#include "api/SuccessAPI.h"

#include "util/Recorder.h"

#include "MouseNativeTouchState.h"

#define DT 1/60.
#define MAGICKEYTIME 0.15

Game* game = 0;
NameInputAPILinuxImpl* nameInput = 0;
Entity globalFTW = 0;

#if SAC_LINUX && ! SAC_EMSCRIPTEN
Recorder *record;
#endif

#if ! SAC_EMSCRIPTEN
std::mutex m;

void GLFWCALL myCharCallback( int c, int action ) {
    if (globalFTW == 0) {

    } else {
        if (TEXT_RENDERING(globalFTW)->show) {
            if (action == GLFW_PRESS && (isalnum(c) || c == ' ')) {
                if (TEXT_RENDERING(globalFTW)->text.length() > 10)
                    return;
                // filter out all unsupported keystrokes
                TEXT_RENDERING(globalFTW)->text.push_back((char)c);
            }
        }
    }
}

void GLFWCALL myKeyCallback( int key, int action ) {
    if (action != GLFW_RELEASE)
        return;
    if (key == GLFW_KEY_BACKSPACE) {
        if (TEXT_RENDERING(nameInput->nameEdit)->show) {
            std::string& text = TEXT_RENDERING(nameInput->nameEdit)->text;
            if (text.length() > 0) {
                text.resize(text.length() - 1);
            }
        } else {
            game->backPressed();
        }
    }
    else if (key == GLFW_KEY_F12) {
        game->togglePause(true);
        uint8_t* out;
        int size = game->saveState(&out);
        if (size) {
            std::ofstream file("/tmp/rr.bin", std::ios_base::binary);
            file.write((const char*)out, size);
            std::cout << "Save state: " << size << " bytes written" << std::endl;
        }
        exit(0);
    }
    else if (key == GLFW_KEY_SPACE ) {// || !focus) {
        if (game->willConsumeBackEvent()) {
            game->backPressed();
        }
    }
}

static void updateAndRenderLoop() {
   bool running = true;

   while(running) {
      game->step();

      running = !glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );

      bool focus = (glfwGetWindowParam(GLFW_ACTIVE) != 0);
      if (focus) {
     theMusicSystem.toggleMute(theSoundSystem.mute);
      } else {
     // theMusicSystem.toggleMute(true);
      }
      //pause ?

#if SAC_LINUX && ! SAC_EMSCRIPTEN
      // recording
      if (glfwGetKey( GLFW_KEY_F10)){
     record->stop();
      }
      if (glfwGetKey( GLFW_KEY_F9)){
     record->start();
      }
#endif
      //user entered his name?
      if (nameInput && glfwGetKey( GLFW_KEY_ENTER )) {
     if (!TEXT_RENDERING(nameInput->nameEdit)->show) {
        nameInput->textIsReady = true;
     }
      }
   }
   theRenderingSystem.disableRendering();
   glfwTerminate();
}

static void* callback_thread(){
    m.lock();
    updateAndRenderLoop();
    m.unlock();
    return NULL;
}

#else
static void updateAndRender() {
    LOGV(1, "gameloop - Pump events");
    SDL_PumpEvents();
    LOGV(1, "gameloop - step");
    game->step();
    LOGV(1, "gameloop - render");
    game->render();
}

#endif

glm::vec2 resolution;
int initGame(const std::string& title) {
    glm::ivec2 reso16_9(394, 700);
    glm::ivec2 reso16_10(900, 625);
    resolution = reso16_10;

    /////////////////////////////////////////////////////
    // Init Window and Rendering
#if SAC_EMSCRIPTEN
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return 1;
    }

    SDL_Surface *ecran = SDL_SetVideoMode(resolution.x, resolution.y, 16, SDL_OPENGL ); /* Double Buffering */
#else
    if (!glfwInit())
        return 1;
    glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE, GL_TRUE );
    if( !(int)glfwOpenWindow(resolution.x, resolution.y, 8,8,8,8,8,8, GLFW_WINDOW ) )
        return 1;
    glfwSetWindowTitle(title.c_str());
    glfwSwapInterval(1);
    glewInit();
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
            std::cout << "Restoring game state from file (size: " << size << ")" << std::endl;
        }
    }
#endif

    /////////////////////////////////////////////////////
    // Game context initialisation
    LOGV(1, "Initialize APIs");
    GameContext ctx;
    if (game->wantsAPI(ContextAPI::Ad))
        ctx.adAPI = new AdAPI();
    if (game->wantsAPI(ContextAPI::Asset) || true)
        ctx.assetAPI = new AssetAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Communication))
        ctx.communicationAPI = new CommunicationAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Exit))
        ctx.exitAPI = new ExitAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Localize))
        ctx.localizeAPI = new LocalizeAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Music))
        ctx.musicAPI = new MusicAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::NameInput))
        ctx.nameInputAPI = new NameInputAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Network))
        ctx.networkAPI = new NetworkAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Sound))
        ctx.soundAPI = new SoundAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::Storage))
        ctx.storageAPI = new StorageAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Success))
        ctx.successAPI = new SuccessAPI();
    if (game->wantsAPI(ContextAPI::Vibrate))
        ctx.vibrateAPI = new VibrateAPILinuxImpl();

    /////////////////////////////////////////////////////
    // Init systems
    theTouchInputManager.setNativeTouchStatePtr(new MouseNativeTouchState());
    theRenderingSystem.assetAPI = ctx.assetAPI;
    if (game->wantsAPI(ContextAPI::Music)) {
        theMusicSystem.musicAPI = ctx.musicAPI;
        theMusicSystem.assetAPI = ctx.assetAPI;
        static_cast<MusicAPILinuxOpenALImpl*>(ctx.musicAPI)->init();
        theMusicSystem.init();
    }
    if (game->wantsAPI(ContextAPI::Sound)) {
        theSoundSystem.soundAPI = ctx.soundAPI;
        static_cast<SoundAPILinuxOpenALImpl*>(ctx.soundAPI)->init(ctx.assetAPI, game->wantsAPI(ContextAPI::Music));
        theSoundSystem.init();
    }
    if (game->wantsAPI(ContextAPI::Localize)) {
        char* lang = strdup(getenv("LANG"));
        lang[2] = '\0';
        static_cast<LocalizeAPILinuxImpl*>(ctx.localizeAPI)->init(ctx.assetAPI, lang);
    }

    /////////////////////////////////////////////////////
    // Init game
    LOGV(1, "Initialize sac & game");
    game->setGameContexts(&ctx, &ctx);
    game->sacInit(resolution.x, resolution.y);
    game->init(state, size);

#if ! SAC_EMSCRIPTEN
    setlocale( LC_ALL, "" );
    // breaks editor -> glfwSetCharCallback(myCharCallback);
    // breaks editor -> glfwSetKeyCallback(myKeyCallback);
#endif


    LOGV(1, "Run game loop");
#if SAC_EMSCRIPTEN
    emscripten_set_main_loop(updateAndRender, 60, 0);
#else
    // record = new Recorder(resolution.x, resolution.y);

    std::thread th1(callback_thread);
    do {
        game->render();
        glfwSwapBuffers();
        // record->record();
    } while (!m.try_lock());
    th1.join();

    delete game;
 //   delete record;
#endif
    return 0;
}

