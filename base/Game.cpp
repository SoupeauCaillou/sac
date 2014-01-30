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



#include "Game.h"

#include "app/MouseNativeTouchState.h"

#include "api/AssetAPI.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "api/StringInputAPI.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"
#include "base/Profiler.h"
#include "base/TouchInputManager.h"
#include "base/JoystickManager.h"

#include "systems/ADSRSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/DebuggingSystem.h"
#include "systems/GraphSystem.h"
#include "systems/GridSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/MusicSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/SpotSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"
#include "systems/SwypeButtonSystem.h"

#include "systems/opengl/OpenGLTextureCreator.h"

#include "util/DataFileParser.h"
#include "util/DrawSomething.h"
#include "util/Recorder.h"
#include "util/Random.h"
#include "util/LevelEditor.h"

#if ! SAC_ANDROID
#include <SDL/SDL.h>
#endif


#include <sstream>

Game::Game() {
#if SAC_INGAME_EDITORS
    gameType = GameType::Default;
#endif
    targetDT = 1.0f / 60.0f;

    isFinished = false;

    mouseNativeTouchState = 0;

    setlocale( LC_NUMERIC, "C" );

    TimeUtil::Init();

    Random::Init();

    /* create EntityManager */
    EntityManager::CreateInstance();

    /* create systems singleton */
    ADSRSystem::CreateInstance();
    AnchorSystem::CreateInstance();
    AnimationSystem::CreateInstance();
    AutoDestroySystem::CreateInstance();
    AutonomousAgentSystem::CreateInstance();
    ButtonSystem::CreateInstance();
    CameraSystem::CreateInstance();
    CollisionSystem::CreateInstance();
    ContainerSystem::CreateInstance();
    DebuggingSystem::CreateInstance();
    GraphSystem::CreateInstance();
    GridSystem::CreateInstance();
    MorphingSystem::CreateInstance();
    MusicSystem::CreateInstance();
    ParticuleSystem::CreateInstance();
    PhysicsSystem::CreateInstance();
    RenderingSystem::CreateInstance();
    ScrollingSystem::CreateInstance();
    SoundSystem::CreateInstance();
    SpotSystem::CreateInstance();
    SpotBlockSystem::CreateInstance();
    TextSystem::CreateInstance();
    TransformationSystem::CreateInstance();
    ZSQDSystem::CreateInstance();
    SwypeButtonSystem::CreateInstance();

#if SAC_NETWORK
    NetworkSystem::CreateInstance();
#endif

    // default
    buildOrderedSystemsToUpdateList();

    fpsStats.reset(0);
    lastUpdateTime = TimeUtil::GetTime();
#if SAC_INGAME_EDITORS
    levelEditor = new LevelEditor();
#endif
}

void Game::buildOrderedSystemsToUpdateList() {
    orderedSystemsToUpdate.clear();

    #define ADD_IF_EXISTING(ptr) do { \
        auto* _ptr = (ptr); \
        if (_ptr) orderedSystemsToUpdate.push_back(_ptr); \
    } while (false)

#if SAC_NETWORK
    ADD_IF_EXISTING(NetworkSystem::GetInstancePointer());
#endif

    ADD_IF_EXISTING(CameraSystem::GetInstancePointer());
    ADD_IF_EXISTING(ADSRSystem::GetInstancePointer());
    ADD_IF_EXISTING(AnimationSystem::GetInstancePointer());
    ADD_IF_EXISTING(AutoDestroySystem::GetInstancePointer());
    ADD_IF_EXISTING(AutonomousAgentSystem::GetInstancePointer());
    ADD_IF_EXISTING(ButtonSystem::GetInstancePointer());
    ADD_IF_EXISTING(CollisionSystem::GetInstancePointer());
    ADD_IF_EXISTING(ContainerSystem::GetInstancePointer());
    ADD_IF_EXISTING(DebuggingSystem::GetInstancePointer());
    ADD_IF_EXISTING(GraphSystem::GetInstancePointer());
    ADD_IF_EXISTING(GridSystem::GetInstancePointer());
    ADD_IF_EXISTING(MorphingSystem::GetInstancePointer());
    ADD_IF_EXISTING(MusicSystem::GetInstancePointer());
    ADD_IF_EXISTING(ParticuleSystem::GetInstancePointer());
    ADD_IF_EXISTING(PhysicsSystem::GetInstancePointer());
    ADD_IF_EXISTING(ScrollingSystem::GetInstancePointer());
    ADD_IF_EXISTING(SoundSystem::GetInstancePointer());
    ADD_IF_EXISTING(SpotSystem::GetInstancePointer());
    ADD_IF_EXISTING(SpotBlockSystem::GetInstancePointer());
    ADD_IF_EXISTING(TextSystem::GetInstancePointer());
    ADD_IF_EXISTING(AnchorSystem::GetInstancePointer());
    ADD_IF_EXISTING(TransformationSystem::GetInstancePointer());
    ADD_IF_EXISTING(ZSQDSystem::GetInstancePointer());
    ADD_IF_EXISTING(SwypeButtonSystem::GetInstancePointer());

#if SAC_ENABLE_LOG
    unusedSystems.clear();
    std::copy(orderedSystemsToUpdate.begin(), orderedSystemsToUpdate.end(), std::inserter(unusedSystems, unusedSystems.begin()));
#endif


    LOGI(orderedSystemsToUpdate.size() << " active systems");
}

Game::~Game() {
    DrawSomething::Clear();

#if !SAC_MOBILE
    JoystickManager::DestroyInstance();
#endif
    EntityManager::DestroyInstance();

    ADSRSystem::DestroyInstance();
    AnchorSystem::DestroyInstance();
    AnimationSystem::DestroyInstance();
    AutoDestroySystem::DestroyInstance();
    AutonomousAgentSystem::DestroyInstance();
    ButtonSystem::DestroyInstance();
    CameraSystem::DestroyInstance();
    CollisionSystem::DestroyInstance();
    ContainerSystem::DestroyInstance();
    DebuggingSystem::DestroyInstance();
    GraphSystem::DestroyInstance();
    GridSystem::DestroyInstance();
    MorphingSystem::DestroyInstance();
    MusicSystem::DestroyInstance();
    ParticuleSystem::DestroyInstance();
    PhysicsSystem::DestroyInstance();
    RenderingSystem::DestroyInstance();
    ScrollingSystem::DestroyInstance();
    SoundSystem::DestroyInstance();
    SpotSystem::DestroyInstance();
    TextSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
    ZSQDSystem::DestroyInstance();
    SwypeButtonSystem::DestroyInstance();

#if SAC_NETWORK
    NetworkSystem::DestroyInstance();
#endif
    orderedSystemsToUpdate.clear();

#if SAC_ENABLE_LOG
    unusedSystems.clear();
#endif
}

void Game::setGameContexts(GameContext* pGameThreadContext, GameContext* pRenderThreadContext) {
    gameThreadContext = pGameThreadContext;
    renderThreadContext = pRenderThreadContext;

    theEntityManager.entityTemplateLibrary.init(gameThreadContext->assetAPI, false);
    theEntityManager.entityTemplateLibrary.setLocalizeAPI(gameThreadContext->localizeAPI);
}

#if SAC_ENABLE_PROFILING
// hum :)
bool profilerEnabled = false;
#endif

void Game::eventsHandler() {
#if ! SAC_ANDROID
    SDL_Event event;
    int handled = 0;

    //if (! (SDL_GetAppState() & SDL_APPINPUTFOCUS)) {
        //LOGI("dont have the focus, dont treat inputs!");
        //return;
    //}
    mouseNativeTouchState->_isMoving = false;

    while( SDL_PollEvent(&event) )
    {
#if SAC_INGAME_EDITORS
        levelEditor->lock();
        // Send event to AntTweakBar
        handled = TwEventSDL(&event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        levelEditor->unlock();
#endif

        //or try stringInputAPI
        if (!handled && wantsAPI(ContextAPI::StringInput)) {
            handled = gameThreadContext->stringInputAPI->eventSDL(&event);
        }
        //or try keyboardAPI
        if (!handled && wantsAPI(ContextAPI::KeyboardInputHandler)) {
            handled = gameThreadContext->keyboardInputHandlerAPI->eventSDL(&event);
        }

        //or try mouse
        if (!handled && mouseNativeTouchState) {
            handled = mouseNativeTouchState->eventSDL(&event);
        }

        //or try joystick
#if !SAC_MOBILE
        if (!handled) {
            handled = JoystickManager::Instance()->eventSDL(&event);
        }
#endif

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

                    switch (key) {
                        case (SDLK_ESCAPE):
                            if (willConsumeBackEvent()) {
                                backPressed();
                            }
                            break;
#if SAC_ENABLE_PROFILING
                        case SDLK_F11:
                            if (profilerEnabled)
                                stopProfiler(
                                    std::string(gameThreadContext->assetAPI->getWritableAppDatasPath() +
                                    "sac_prof.json").c_str());
                            else
                                startProfiler();
                            profilerEnabled = !profilerEnabled;
                            break;
#endif
                        case SDLK_F9:
#if SAC_LINUX && SAC_DESKTOP
                            Recorder::Instance().toggle();
#endif
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
#endif
}

void Game::loadFont(AssetAPI* asset, const std::string& name) {
    FileBuffer file = asset->loadAsset(name + ".font");
    DataFileParser dfp;
    if (!dfp.load(file, name + ".font")) {
        LOGE("Invalid font description file: " << name);
        return;
    }

    unsigned defCount = dfp.sectionSize(DataFileParser::GlobalSection);
    LOGW_IF(defCount == 0, "Font definition '" << name << "' has no entry");
    std::map<uint32_t, float> h2wratio;
    std::string charUnicode;
    for (unsigned i=0; i<defCount; i++) {
        int w_h[2];
        if (!dfp.get(DataFileParser::GlobalSection, i, charUnicode, w_h, 2)) {
            LOGE("Unable to parse entry #" << i << " of " << name);
        }
        std::stringstream ss;
        ss << std::hex << charUnicode;
        uint32_t cId;
        ss >> cId;
        h2wratio[cId] = (float)w_h[0] / w_h[1];
        LOGV(2, "Font entry: " << cId << ": " << h2wratio[cId]);
    }
    delete[] file.data;
    // h2wratio[' '] = h2wratio['r'];
    // h2wratio[0x97] = 1;
    theTextSystem.registerFont(name, h2wratio);
    LOGI("Loaded font: " << name << ". Found: " << h2wratio.size() << " entries");
}

void Game::sacInit(int windowW, int windowH) {
#if SAC_ENABLE_PROFILING
    initProfiler();
#endif

    if (windowW < windowH) {
        PlacementHelper::ScreenSize = glm::vec2(10.f * windowW / (float)windowH, 10.f);
    } else {
        PlacementHelper::ScreenSize = glm::vec2(20.f, 20.f * windowH / (float)windowW);
    }

    PlacementHelper::WindowSize = glm::vec2(windowW, windowH);
    PlacementHelper::GimpSize = glm::vec2(800.0f, 1280.0f);


    theRenderingSystem.setWindowSize(PlacementHelper::WindowSize, PlacementHelper::ScreenSize);
    theTouchInputManager.init(PlacementHelper::ScreenSize, PlacementHelper::WindowSize);

    theRenderingSystem.init();

    // Auto-load all atlas
    {
        const std::string dpiFolder(OpenGLTextureCreator::DPI2Folder(OpenGLTextureCreator::dpi));
        std::list<std::string> atlas = renderThreadContext->assetAPI->listAssetContent(
            ".atlas", dpiFolder);
        LOGI("Autoloading " << atlas.size() << " atlas");
        std::for_each(atlas.begin(), atlas.end(), [dpiFolder] (const std::string& a) -> void {
            theRenderingSystem.loadAtlas(dpiFolder + '/' + a);
        });
    }

    // Auto-load all fonts
    {
        std::list<std::string> fonts = renderThreadContext->assetAPI->listAssetContent(
            ".font");
        LOGI("Autoloading " << fonts.size() << " fonts");
        std::for_each(fonts.begin(), fonts.end(), [this] (const std::string& typo) -> void {
            loadFont(renderThreadContext->assetAPI, typo);
        });

    }

#if SAC_INGAME_EDITORS
    levelEditor->init();
#endif
}

int Game::saveState(uint8_t**) {
    return 0;
}

void Game::step() {
    PROFILE("Game", "step", BeginEvent);

    theEntityManager.entityTemplateLibrary.update();

#if SAC_DESKTOP    
    theEntityManager.entityTemplateLibrary.updateReload();
#endif

    theRenderingSystem.waitDrawingComplete();

    theTouchInputManager.Update();

#if !SAC_MOBILE
    theJoystickManager.Update();
#endif
    float timeBeforeThisStep = TimeUtil::GetTime();
    float delta = timestep.smooth(timeBeforeThisStep - lastUpdateTime);

#if SAC_ENABLE_PROFILING
    std::stringstream framename;
    framename << "update-" << (int)(delta * 1000000);
    PROFILE("Game", framename.str(), InstantEvent);
#endif

    // update game state
#if SAC_INGAME_EDITORS
    static float speedFactor = 1.0f;
    static bool oneStepEnabled = false;

    Uint8 *keystate = SDL_GetKeyState(NULL);
    // Always tick levelEditor (manages AntTweakBar stuff)
    PROFILE("Game", "AntTweakBar", BeginEvent);
    levelEditor->tick(delta);
    PROFILE("Game", "AntTweakBar", EndEvent);

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
            LOGI("Single stepping the game (delta: " << delta << " ms)");
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
    LOGV(1, "Update game");
    tick(delta);
#endif

#if SAC_INGAME_EDITORS
    if (delta > 0) {
#endif

    #if SAC_ENABLE_LOG
        static int timer = 0;
        //every 20 secs, log it
        if (++timer == 60*20) {
            timer = 0;
            for (auto* sys : unusedSystems) {
                LOGW("System " << sys->getName() << " has (yet) not been used");
            }
        }
    #endif

    LOGV(2, "Update systems");

    for (auto* sys : orderedSystemsToUpdate) {
        #if SAC_ENABLE_LOG
            //if system contains entities, remove it from "unused" systems set
            if (sys->entityCount()) {
                std::set<ComponentSystem*>::iterator systemIt;
                if ((systemIt = unusedSystems.find(sys)) != unusedSystems.end()) {
                    unusedSystems.erase(systemIt);
                }
            }
        #endif
        sys->Update(delta);
    }
#if SAC_INGAME_EDITORS
    } else {
        theAnchorSystem.Update(delta);
    }
    if (oneStepEnabled) {
        LOGI("one more step");
        oneStepEnabled = false;
        speedFactor = 0.;
    }

#endif
    LOGV(2, "Produce rendering frame");
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

#if SAC_DEBUG
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
                (1000.0 / (t - fpsStats.since)) << '/' << (1.0 / fpsStats.maxDt) << '/' << (1.0 / fpsStats.minDt));
            count = 0;
            fpsStats.reset(t);
        }
    }
#endif
    PROFILE("Game", "render-game", EndEvent);
}

void Game::resetTime() {
    fpsStats.reset(TimeUtil::GetTime());
    lastUpdateTime = TimeUtil::GetTime();
}
