#include "Game.h"

#include <base/EntityManager.h>
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
#include "systems/ScrollingSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/CameraSystem.h"
#include "systems/GraphSystem.h"
#include "api/AssetAPI.h"
#include "base/PlacementHelper.h"
#include "base/TouchInputManager.h"
#include "base/Profiler.h"
#include "util/DataFileParser.h"
#include <sstream>

#ifdef INGAME_EDITORS
#include <GL/glfw.h>
#endif

Game::Game() {
#ifdef INGAME_EDITORS
    gameType = GameType::Default;
#endif
    targetDT = 1.0 / 60.0;

	/* create EntityManager */
	EntityManager::CreateInstance();

	/* create systems singleton */
	TransformationSystem::CreateInstance();
	RenderingSystem::CreateInstance();
	SoundSystem::CreateInstance();
    MusicSystem::CreateInstance();
	ADSRSystem::CreateInstance();
	ButtonSystem::CreateInstance();
	TextRenderingSystem::CreateInstance();
	ContainerSystem::CreateInstance();
	PhysicsSystem::CreateInstance();
    ParticuleSystem::CreateInstance();
    ScrollingSystem::CreateInstance();
    MorphingSystem::CreateInstance();
    AutonomousAgentSystem::CreateInstance();
    AnimationSystem::CreateInstance();
    AutoDestroySystem::CreateInstance();
    CameraSystem::CreateInstance();
    GraphSystem::CreateInstance();

#ifdef SAC_NETWORK
    NetworkSystem::CreateInstance();
#endif

    fpsStats.reset(0);
    lastUpdateTime = TimeUtil::getTime();
#ifdef INGAME_EDITORS
    levelEditor = new LevelEditor();
#endif
}

Game::~Game() {
    EntityManager::DestroyInstance();
    TransformationSystem::DestroyInstance();
    RenderingSystem::DestroyInstance();
    SoundSystem::DestroyInstance();
    MusicSystem::DestroyInstance();
    ADSRSystem::DestroyInstance();
    ButtonSystem::DestroyInstance();
    TextRenderingSystem::DestroyInstance();
    ContainerSystem::DestroyInstance();
    PhysicsSystem::DestroyInstance();
    ParticuleSystem::DestroyInstance();
    ScrollingSystem::DestroyInstance();
    MorphingSystem::DestroyInstance();
    AutonomousAgentSystem::DestroyInstance();
    AnimationSystem::DestroyInstance();
    AutoDestroySystem::DestroyInstance();
    CameraSystem::DestroyInstance();
    GraphSystem::DestroyInstance();

#ifdef SAC_NETWORK
    NetworkSystem::DestroyInstance();
#endif
}

void Game::loadFont(AssetAPI* asset, const std::string& name) {
	FileBuffer file = asset->loadAsset(name + ".desc");
    DataFileParser dfp;
    if (!dfp.load(file)) {
        LOG(ERROR) << "Invalid font description file: " << name;
        return;
    }
    
    unsigned defCount = dfp.sectionSize(DataFileParser::GlobalSection);
    LOG_IF(WARNING, defCount == 0) << "Font definition '" << name << "' has no entry";
    std::map<uint32_t, float> h2wratio;
    std::string charUnicode;
    for (unsigned i=0; i<defCount; i++) {
        int w_h[2];
        if (!dfp.get(DataFileParser::GlobalSection, i, charUnicode, w_h, 2)) {
            LOG(ERROR) << "Unable to parse entry #" << i << " of " << name;
        }
        std::stringstream ss;
        ss << std::hex << charUnicode;
        uint32_t cId;
        ss >> cId;
        h2wratio[cId] = (float)w_h[0] / w_h[1];
        VLOG(2) << "Font entry: " << cId << ": " << h2wratio[cId];
    }
	delete[] file.data;
	// h2wratio[' '] = h2wratio['r'];
	// h2wratio[0x97] = 1;
	theTextRenderingSystem.registerFont(name, h2wratio);
    LOG(INFO) << "Loaded font: " << name << ". Found: " << h2wratio.size() << " entries";
}

void Game::sacInit(int windowW, int windowH) {
#ifdef ENABLE_PROFILING
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

	theRenderingSystem.setWindowSize(windowW, windowH, PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight);
	theTouchInputManager.init(Vector2(PlacementHelper::ScreenWidth, PlacementHelper::ScreenHeight), Vector2(windowW, windowH));

	theRenderingSystem.init();
    theRenderingSystem.setFrameQueueWritable(true);
}

void Game::backPressed() {
	#ifdef ENABLE_PROFILING
	static int profStarted = 0;
	if ((profStarted % 2) == 0) {
		startProfiler();
	} else {
		std::stringstream a;
		#ifdef ANDROID
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
    theRenderingSystem.setFrameQueueWritable(false);
	return 0;
}

const float DDD = 1.0/60.f;
void Game::step() {
    PROFILE("Game", "step", BeginEvent);

    theRenderingSystem.waitDrawingComplete();

    float t = TimeUtil::getTime();
    float delta = t - lastUpdateTime;

    if (true) {
        #if 0
        if (delta < DDD) {
            t += (DDD - delta);
            delta = DDD;
        }
        #endif

        theTouchInputManager.Update(delta);
        #ifdef ENABLE_PROFILING
        std::stringstream framename;
        framename << "update-" << (int)(delta * 1000000);
        PROFILE("Game", framename.str(), InstantEvent);
        #endif
        // delta = 1.0 / 60;
        // update game state
        #ifdef INGAME_EDITORS
        static float speedFactor = 1.0f;
        if (glfwGetKey(GLFW_KEY_F1))
            gameType = GameType::Default;
        else if (glfwGetKey(GLFW_KEY_F2))
            gameType = GameType::LevelEditor;
        switch (gameType) {
            case GameType::LevelEditor:
                levelEditor->tick(delta);
                delta = 0;
                break;
            default:
                if (/*glfwGetKey(GLFW_KEY_KP_ADD) ||*/ glfwGetKey(GLFW_KEY_F6)) {
                    speedFactor += 1 * delta;
                } else if (/*glfwGetKey(GLFW_KEY_KP_SUBTRACT) ||*/ glfwGetKey(GLFW_KEY_F5)) {
                    speedFactor = MathUtil::Max(speedFactor - 1 * delta, 0.0f);
                } else if (glfwGetKey(GLFW_KEY_KP_ENTER)) {
                    speedFactor = 1;
                }
                delta *= speedFactor;
                tick(delta);
        }
        #else
        tick(delta);
        #endif

        #ifdef INGAME_EDITORS
        if (delta > 0) {
        #endif
        #ifdef SAC_NETWORK
        theNetworkSystem.Update(delta);
        #endif

        theCameraSystem.Update(delta);
        theADSRSystem.Update(delta);
        theAnimationSystem.Update(delta);
        theButtonSystem.Update(delta);
        theAutonomousAgentSystem.Update(delta);
        theMorphingSystem.Update(delta);
        thePhysicsSystem.Update(delta);
        theScrollingSystem.Update(delta);
        theSoundSystem.Update(delta);
        theMusicSystem.Update(delta);
        theTextRenderingSystem.Update(delta);
        theTransformationSystem.Update(delta);
        theParticuleSystem.Update(delta);
        theContainerSystem.Update(delta);
        theAutoDestroySystem.Update(delta);
        theGraphSystem.Update(delta);
        #ifdef INGAME_EDITORS
        } else {
            theTransformationSystem.Update(delta);
        }
        #endif
        // produce 1 new frame
        theRenderingSystem.Update(0);

        lastUpdateTime = t;
        delta = TimeUtil::getTime() - t;
        #ifndef EMSCRIPTEN
        while (delta < 0.016) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (0.016 - delta) * 1000000000LL;
            nanosleep(&ts, 0);
            delta = TimeUtil::getTime() - t;
        }
        #endif
    } else {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (0.008 - delta) * 1000000000LL;
        nanosleep(&ts, 0);
    }
    PROFILE("Game", "step", EndEvent);
}

void Game::render() {
    PROFILE("Game", "render-game", BeginEvent);
    theRenderingSystem.render();

    {
        static float prevT = 0;
        float t = TimeUtil::getTime();
        float dt = t - prevT;
        prevT = t;

        if (dt > fpsStats.maxDt) {
            fpsStats.maxDt = dt;
        }
        if (dt < fpsStats.minDt) {
            fpsStats.minDt = dt;
        }
        LOG_EVERY_N(INFO, 1000) << "FPS avg/min/max : " <<
            (1000 / (t - fpsStats.since)) << '/' << (1.0 / fpsStats.maxDt) << '/' << (1.0 / fpsStats.minDt), 
        fpsStats.reset(t);
    }

    if (0) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (0.016) * 1000000000LL;
        nanosleep(&ts, 0);
    }
    PROFILE("Game", "render-game", EndEvent);
}

void Game::resetTime() {
    fpsStats.reset(TimeUtil::getTime());
    lastUpdateTime = TimeUtil::getTime();
}
