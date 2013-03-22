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
#define GLOG_NO_ABBREVIATED_SEVERITIES

#include "base/Game.h"
#include "base/GameContext.h"

#ifdef EMSCRIPTEN
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

#ifndef EMSCRIPTEN
	#ifdef WINDOWS

	#else
		#include <locale.h>
		#include <libintl.h>
	#endif
#endif

#include "base/Vector2.h"
#include "base/TouchInputManager.h"
#include "base/TimeUtil.h"
#include "base/MathUtil.h"
#include "base/PlacementHelper.h"
#include "base/Profiler.h"
#include "systems/RenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"
#include "api/AdAPI.h"
#include "api/linux/MusicAPILinuxOpenALImpl.h"
#include "api/linux/AssetAPILinuxImpl.h"
#include "api/linux/SoundAPILinuxOpenALImpl.h"
#include "api/linux/LocalizeAPILinuxImpl.h"
#include "api/linux/NameInputAPILinuxImpl.h"
#include "api/linux/ExitAPILinuxImpl.h"
#include "api/linux/NetworkAPILinuxImpl.h"
#include "api/linux/CommunicationAPILinuxImpl.h"
#include "api/linux/VibrateAPILinuxImpl.h"
#include "api/SuccessAPI.h"
#include "util/Recorder.h"

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "MouseNativeTouchState.h"

#define DT 1/60.
#define MAGICKEYTIME 0.15

Game* game;
NameInputAPILinuxImpl* nameInput;
Entity globalFTW = 0;

#if defined(LINUX) && !defined(EMSCRIPTEN)
Recorder *record;
#endif

#if !defined(EMSCRIPTEN)
std::mutex m;

void GLFWCALL myCharCallback( int c, int action ) {
    if (globalFTW == 0) {

    } else {
        if (!TEXT_RENDERING(globalFTW)->hide) {
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
        if (!TEXT_RENDERING(nameInput->nameEdit)->hide) {
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

      #if defined(LINUX) && !defined(EMSCRIPTEN)
      // recording
      if (glfwGetKey( GLFW_KEY_F10)){
     record->stop();
      }
      if (glfwGetKey( GLFW_KEY_F9)){
     record->start();
      }
#endif
      //user entered his name?
      if (glfwGetKey( GLFW_KEY_ENTER )) {
     if (!TEXT_RENDERING(nameInput->nameEdit)->hide) {
        nameInput->textIsReady = true;
     }
      }
   }
   theRenderingSystem.setFrameQueueWritable(false);
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
    SDL_PumpEvents();
    game->step();
    game->render();
}

#endif

/** Engine entry point */
int launchGame(const std::string& title, Game* gameImpl, unsigned contextOptions, int argc, char** argv) {
#ifdef WINDOWS
	LOG(WARNING) << "TODO";
#else
	google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    FLAGS_colorlogtostderr = true;
    google::ParseCommandLineFlags(&argc, &argv, true);
#endif

    Vector2 reso16_9(394, 700);
    Vector2 reso16_10(900, 625);
    Vector2* reso = &reso16_10;
    TimeUtil::Init();
    
    game = gameImpl;

    /////////////////////////////////////////////////////
    // Init Window and Rendering
#ifdef EMSCRIPTEN
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        return 1;
    }

    SDL_Surface *ecran = SDL_SetVideoMode(reso->X, reso->Y, 16, SDL_OPENGL ); /* Double Buffering */
#else
    if (!glfwInit())
        return 1;
    glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE, GL_TRUE );
    if( !(int)glfwOpenWindow((int)reso->X, (int)reso->Y, 8,8,8,8,8,8, GLFW_WINDOW ) )
        return 1;
    glfwSetWindowTitle(title.c_str());
    glewInit();
    bool restore = false;
    for (int i=1; i<argc; i++) {
        restore |= !strcmp(argv[i], "-restore");
    }
#endif

    /////////////////////////////////////////////////////
    // Handle --restore cmd line switch
    uint8_t* state = 0;
    int size = 0;
    #ifndef EMSCRIPTEN
    if (restore) {
        // TODO: portability
        std::stringstream restoreFile;
        restoreFile << "/tmp/" << title << ".bin";
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
    GameContext ctx;
    if (contextOptions & CONTEXT_WANT_AD_API)
        ctx.adAPI = new AdAPI();
    if (contextOptions & CONTEXT_WANT_ASSET_API || true)
        ctx.assetAPI = new AssetAPILinuxImpl();
    if (contextOptions & CONTEXT_WANT_COMM_API)
        ctx.communicationAPI = new CommunicationAPILinuxImpl();
    if (contextOptions & CONTEXT_WANT_EXIT_API)
        ctx.exitAPI = new ExitAPILinuxImpl();
    if (contextOptions & CONTEXT_WANT_LOCALIZE_API)
        ctx.localizeAPI = new LocalizeAPILinuxImpl();
    if (contextOptions & CONTEXT_WANT_MUSIC_API)
        ctx.musicAPI = new MusicAPILinuxOpenALImpl();
    if (contextOptions & CONTEXT_WANT_NAME_INPUT_API)
        ctx.nameInputAPI = new NameInputAPILinuxImpl();
    if (contextOptions & CONTEXT_WANT_NETWORK_API)
        ctx.networkAPI = new NetworkAPILinuxImpl();
    if (contextOptions & CONTEXT_WANT_SOUND_API)
        ctx.soundAPI = new SoundAPILinuxOpenALImpl();
    if (contextOptions & CONTEXT_WANT_SUCCESS_API)
        ctx.successAPI = new SuccessAPI();
    if (contextOptions & CONTEXT_WANT_VIBRATE_API)
        ctx.vibrateAPI = new VibrateAPILinuxImpl();

    /////////////////////////////////////////////////////
    // Init systems
    theTouchInputManager.setNativeTouchStatePtr(new MouseNativeTouchState());
    theRenderingSystem.assetAPI = ctx.assetAPI;
    if (contextOptions & CONTEXT_WANT_SOUND_API) {
        theSoundSystem.soundAPI = ctx.soundAPI;
        theSoundSystem.init();
    }
    if (contextOptions & CONTEXT_WANT_MUSIC_API) {
        theMusicSystem.musicAPI = ctx.musicAPI;
        theMusicSystem.assetAPI = ctx.assetAPI;
        theMusicSystem.init();
    }
    

    /////////////////////////////////////////////////////
    // Init game
    game->setGameContexts(&ctx, &ctx);
    game->sacInit((int)reso->X, (int)reso->Y);
    game->init(state, size);

#ifndef EMSCRIPTEN
    setlocale( LC_ALL, "" );
    // breaks editor -> glfwSetCharCallback(myCharCallback);
    // breaks editor -> glfwSetKeyCallback(myKeyCallback);
#endif

#ifndef EMSCRIPTEN
    // record = new Recorder((int)reso->X, (int)reso->Y);

    std::thread th1(callback_thread);    
    do {
        game->render();
        glfwSwapBuffers();
        // record->record();
    } while (!m.try_lock());
    th1.join();
#else
    emscripten_set_main_loop(updateAndRender, 60, 0);
#endif

#ifndef EMSCRIPTEN
    delete game;
 //   delete record;
#endif
    return 0;
}

