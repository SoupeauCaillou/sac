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
#include "api/AssetAPI.h"
#include "base/PlacementHelper.h"
#include "base/Profiler.h"
#include <sstream>

Game::Game() {
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
#ifdef SAC_NETWORK
    NetworkSystem::CreateInstance();
#endif

    fpsStats.reset(0);
    lastUpdateTime = TimeUtil::getTime();
}

Game::~Game() {
    LOGW("Delete game instance %p", this);
    theEntityManager.deleteAllEntities();
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
#ifdef SAC_NETWORK
    NetworkSystem::DestroyInstance();
#endif
}

void Game::loadFont(AssetAPI* asset, const std::string& name) {
	FileBuffer file = asset->loadAsset(name + ".desc");
	std::stringstream sfont;
	sfont << std::string((char*)file.data, file.size);
	std::string line;
	std::map<unsigned char, float> h2wratio;
	while (getline(sfont, line)) {
		if (line[0] == '#')
			continue;
		int c, w, h;
		sscanf(line.c_str(), "%d,%d,%d", &c, &w, &h);
		h2wratio[c] = (float)w / h;
	}
 
	delete[] file.data;
	h2wratio[' '] = h2wratio['r'];
	h2wratio[0x97] = 1;
	theTextRenderingSystem.registerFont(name, h2wratio);
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
		
void Game::togglePause(bool activate __attribute__((unused))) {}
		
int Game::saveState(uint8_t** out __attribute__((unused))) {
	return 0;
}

void Game::step() {
    PROFILE("Game", "step", BeginEvent);
    theRenderingSystem.waitDrawingComplete();

    const float t = TimeUtil::getTime();
    float delta = t - lastUpdateTime;
    if (true || delta > 0.012) {
        // update game state
        tick(delta);
        // produce 1 new frame
        theRenderingSystem.Update(0);

        lastUpdateTime = t;
        delta = TimeUtil::getTime() - t;
    } else {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (0.012 - delta) * 1000000000LL;
        nanosleep(&ts, 0);
        delta = TimeUtil::getTime() - t;
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

        fpsStats.frameCount++;
        if (dt > fpsStats.maxDt)
            fpsStats.maxDt = dt;
        if (dt < fpsStats.minDt) {
            fpsStats.minDt = dt;
        }
        if (fpsStats.frameCount == 1000) {
            LOGW("FPS avg/min/max : %.2f / %.2f / %.2f",
                fpsStats.frameCount / (t - fpsStats.since), 1.0 / fpsStats.maxDt, 1.0 / fpsStats.minDt);
            fpsStats.reset(t);
        }
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
