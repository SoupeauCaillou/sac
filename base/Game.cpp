#include "Game.h"

#include <base/EntityManager.h>
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/ZSQDSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/CameraSystem.h"
#include "systems/GraphSystem.h"
#include "systems/DebuggingSystem.h"
#include "api/AssetAPI.h"
#include "base/PlacementHelper.h"
#include "base/TouchInputManager.h"
#include "base/Profiler.h"
#include "util/DataFileParser.h"
#include <sstream>

#include <SDL/SDL.h>

#include "api/KeyboardInputHandlerAPI.h"
#include "app/MouseNativeTouchState.h"

Game::Game() {
#if SAC_INGAME_EDITORS
    gameType = GameType::Default;
#endif
    targetDT = 1.0f / 60.0f;

    isFinished = false;

    mouseNativeTouchState = 0;

    TimeUtil::Init();

	/* create EntityManager */
	EntityManager::CreateInstance();

	/* create systems singleton */
    AnchorSystem::CreateInstance();
	TransformationSystem::CreateInstance();
	RenderingSystem::CreateInstance();
	SoundSystem::CreateInstance();
    MusicSystem::CreateInstance();
	ADSRSystem::CreateInstance();
	ButtonSystem::CreateInstance();
	TextRenderingSystem::CreateInstance();
	ContainerSystem::CreateInstance();
    PhysicsSystem::CreateInstance();
    ZSQDSystem::CreateInstance();
    ParticuleSystem::CreateInstance();
    ScrollingSystem::CreateInstance();
    MorphingSystem::CreateInstance();
    AutonomousAgentSystem::CreateInstance();
    AnimationSystem::CreateInstance();
    AutoDestroySystem::CreateInstance();
    CameraSystem::CreateInstance();
    GraphSystem::CreateInstance();
    DebuggingSystem::CreateInstance();

#if SAC_NETWORK
    NetworkSystem::CreateInstance();
#endif

    fpsStats.reset(0);
    lastUpdateTime = TimeUtil::GetTime();
#if SAC_INGAME_EDITORS
    levelEditor = new LevelEditor();
#endif
}

Game::~Game() {
    EntityManager::DestroyInstance();
    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
    RenderingSystem::DestroyInstance();
    SoundSystem::DestroyInstance();
    MusicSystem::DestroyInstance();
    ADSRSystem::DestroyInstance();
    ButtonSystem::DestroyInstance();
    TextRenderingSystem::DestroyInstance();
    ContainerSystem::DestroyInstance();
    ZSQDSystem::DestroyInstance();
    PhysicsSystem::DestroyInstance();
    ParticuleSystem::DestroyInstance();
    ScrollingSystem::DestroyInstance();
    MorphingSystem::DestroyInstance();
    AutonomousAgentSystem::DestroyInstance();
    AnimationSystem::DestroyInstance();
    AutoDestroySystem::DestroyInstance();
    CameraSystem::DestroyInstance();
    GraphSystem::DestroyInstance();
	DebuggingSystem::DestroyInstance();

#if SAC_NETWORK
    NetworkSystem::DestroyInstance();
#endif
}

void Game::setGameContexts(GameContext* pGameThreadContext, GameContext* pRenderThreadContext) {
    gameThreadContext = pGameThreadContext;
    renderThreadContext = pRenderThreadContext;

    theEntityManager.entityTemplateLibrary.init(gameThreadContext->assetAPI, false);
}

void Game::eventsHandler() {
    SDL_Event event;
    int handled = 0;

    //if (! (SDL_GetAppState() & SDL_APPINPUTFOCUS)) {
        //LOGI("dont have the focus, dont treat inputs!");
        //return;
    //}

    while( SDL_PollEvent(&event) )
    {
#if SAC_INGAME_EDITORS
        // Send event to AntTweakBar
        handled = TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
#endif

        //or try keyboardAPI
        if (!handled && wantsAPI(ContextAPI::KeyboardInputHandler)) {
            handled = gameThreadContext->keyboardInputHandlerAPI->eventSDL(&event);
        }

        //or try mouse
        if (!handled && mouseNativeTouchState) {
            handled = mouseNativeTouchState->eventSDL(&event);
        }

        // If event has not been handled by anyone, treat it
        if( !handled )
        {
            switch( event.type )
            {
                case SDL_QUIT: {
                    isFinished = true;
                    break;
                }
                case SDL_KEYUP: {
                    int key = event.key.keysym.sym;
                    LOGI("key released: " << key);

                    switch (key) {
                        case (SDLK_ESCAPE):
                            isFinished = true;
                            break;

#if SAC_INGAME_EDITORS
                        //if we use the editor; we need to handle some keys for it
                        case (SDLK_F4):
                            theDebuggingSystem.toggle();
                            break;
#endif
                    }
                }
            }
        }
    }
    if (wantsAPI(ContextAPI::KeyboardInputHandler)) {
        gameThreadContext->keyboardInputHandlerAPI->update();
    }
}

void Game::loadFont(AssetAPI* asset, const std::string& name) {
	FileBuffer file = asset->loadAsset(name + ".font");
    DataFileParser dfp;
    if (!dfp.load(file, name + ".font")) {
        LOGE("Invalid font description file: " << name)
        return;
    }

    unsigned defCount = dfp.sectionSize(DataFileParser::GlobalSection);
    LOGW_IF(defCount == 0, "Font definition '" << name << "' has no entry")
    std::map<uint32_t, float> h2wratio;
    std::string charUnicode;
    for (unsigned i=0; i<defCount; i++) {
        int w_h[2];
        if (!dfp.get(DataFileParser::GlobalSection, i, charUnicode, w_h, 2)) {
            LOGE("Unable to parse entry #" << i << " of " << name)
        }
        std::stringstream ss;
        ss << std::hex << charUnicode;
        uint32_t cId;
        ss >> cId;
        h2wratio[cId] = (float)w_h[0] / w_h[1];
        LOGV(2, "Font entry: " << cId << ": " << h2wratio[cId])
    }
	delete[] file.data;
	// h2wratio[' '] = h2wratio['r'];
	// h2wratio[0x97] = 1;
	theTextRenderingSystem.registerFont(name, h2wratio);
    LOGI("Loaded font: " << name << ". Found: " << h2wratio.size() << " entries")
}

void Game::sacInit(int windowW, int windowH) {
#if SAC_ENABLE_PROFILING
	initProfiler();
#endif

    if (windowW < windowH) {
    	    PlacementHelper::ScreenHeight = 10;
        PlacementHelper::ScreenWidth = PlacementHelper::ScreenHeight * windowW / (float)windowH;
    } else {
        PlacementHelper::ScreenWidth = 20;
        PlacementHelper::ScreenHeight = PlacementHelper::ScreenWidth * windowH / (float)windowW;
    }

    PlacementHelper::WindowWidth = windowW;
    PlacementHelper::WindowHeight = windowH;
    PlacementHelper::GimpWidth = 800.0f;
    PlacementHelper::GimpHeight = 1280.0f;

    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	theRenderingSystem.setWindowSize(windowW, windowH, PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
	theTouchInputManager.init(glm::vec2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight), glm::vec2(windowW, windowH));

	theRenderingSystem.init();
    theRenderingSystem.enableRendering();
}

void Game::backPressed() {
#if SAC_ENABLE_PROFILING
	static int profStarted = 0;
	if ((profStarted % 2) == 0) {
		startProfiler();
	} else {
		std::stringstream a;
#if SAC_ANDROID
		a << "/sdcard/";
#else
		a << "/tmp/";
#endif
		a << "sac_prof_" << (int)(profStarted / 2) << ".json";
		stopProfiler(a.str());
	}
	profStarted++;
#endif
}

int Game::saveState(uint8_t**) {
	return 0;
}

const float DDD = 1.0/60.f;

void Game::step() {
    PROFILE("Game", "step", BeginEvent);

    theRenderingSystem.waitDrawingComplete();

    float timeBeforeThisStep = TimeUtil::GetTime();
    float delta = timeBeforeThisStep - lastUpdateTime;

    LOGV(2, "dt = " << delta)
    LOGV(2, "Update input")
    theTouchInputManager.Update(delta);
#if SAC_ENABLE_PROFILING
    std::stringstream framename;
    framename << "update-" << (int)(delta * 1000000);
    PROFILE("Game", framename.str(), InstantEvent);
#endif
    theEntityManager.entityTemplateLibrary.update();
    theEntityManager.entityTemplateLibrary.updateReload();

    // update game state
#if SAC_INGAME_EDITORS
    static float speedFactor = 1.0f;
    static bool oneStepEnabled = false;

    Uint8 *keystate = SDL_GetKeyState(NULL);
    // Always tick levelEditor (manages AntTweakBar stuff)
    levelEditor->tick(delta);

    if (keystate[SDLK_F1])
        gameType = GameType::Default;
    else if (keystate[SDLK_F2])
        gameType = GameType::LevelEditor;
    else if (keystate[SDLK_F3])
        gameType = GameType::SingleStep;

    switch (gameType) {
        case GameType::LevelEditor:
            delta = 0;
            break;
        case GameType::SingleStep:
            delta = 1.0/60;
            LOGI("Single stepping the game (delta: " << delta << " ms)")
            tick(delta);
            gameType = GameType::LevelEditor;
        default:
            if (/*keystate[SDLK_KP_SUBTRACT] ||*/ keystate[SDLK_F5]) {
                speedFactor = glm::max(speedFactor - 1 * delta, 0.0f);
            } else if (/*keystate[SDLK_KP_ADD] ||*/ keystate[SDLK_F6]) {
                speedFactor += 1 * delta;
            } else if (/*keystate[SDLK_KP_SUBTRACT] ||*/ keystate[SDLK_F7]) {
                oneStepEnabled = true;
                levelEditor->tick(delta);
                speedFactor = 1;
            } else if (keystate[SDLK_KP_ENTER]) {
                speedFactor = 1;
            }
            delta *= speedFactor;
            tick(delta);
    }
#else
    LOGV(1, "Update game")
    tick(delta);
#endif

#if SAC_INGAME_EDITORS
    if (delta > 0) {
#endif

#if SAC_NETWORK
    theNetworkSystem.Update(delta);
#endif

    LOGV(2, "Update systems");
    theCameraSystem.Update(delta);
    theADSRSystem.Update(delta);
    theAnimationSystem.Update(delta);
    theButtonSystem.Update(delta);
    theAutonomousAgentSystem.Update(delta);
    theMorphingSystem.Update(delta);
    thePhysicsSystem.Update(delta);
    theScrollingSystem.Update(delta);
    theZSQDSystem.Update(delta);
    theSoundSystem.Update(delta);
    theMusicSystem.Update(delta);
    theTextRenderingSystem.Update(delta);
    theAnchorSystem.Update(delta);
    theTransformationSystem.Update(delta);
    theParticuleSystem.Update(delta);
    theContainerSystem.Update(delta);
    theAutoDestroySystem.Update(delta);
    theDebuggingSystem.Update(delta);
    theGraphSystem.Update(delta);
#if SAC_INGAME_EDITORS
    } else {
        theAnchorSystem.Update(delta);
    }
    if (oneStepEnabled) {
        LOGI("one more step")
        oneStepEnabled = false;
        speedFactor = 0.;
    }

#endif
    LOGV(2, "Produce rendering frame")
    // produce 1 new frame
    theRenderingSystem.Update(0);

    float updateDuration = TimeUtil::GetTime() - timeBeforeThisStep;
    if (updateDuration < 0.016) {
        //TimeUtil::Wait(0.016 - updateDuration);
    }
    lastUpdateTime = timeBeforeThisStep;

    PROFILE("Game", "step", EndEvent);
}

void Game::render() {
    PROFILE("Game", "render-game", BeginEvent);
    theRenderingSystem.render();

    {
        static int count = 0;
        static float prevT = 0;
        float t = TimeUtil::GetTime();
        float dt = t - prevT;
        prevT = t;

        if (dt > fpsStats.maxDt) {
            fpsStats.maxDt = dt;
        }
        if (dt < fpsStats.minDt) {
            fpsStats.minDt = dt;
        }
        ++count;
        if (count == 1000) {
            LOGV(LogVerbosity::VERBOSE1, "FPS avg/min/max : " <<
                (1000.0 / (t - fpsStats.since)) << '/' << (1.0 / fpsStats.maxDt) << '/' << (1.0 / fpsStats.minDt))
            count = 0;
            fpsStats.reset(t);
        }
    }
    PROFILE("Game", "render-game", EndEvent);
}

void Game::resetTime() {
    fpsStats.reset(TimeUtil::GetTime());
    lastUpdateTime = TimeUtil::GetTime();
}
