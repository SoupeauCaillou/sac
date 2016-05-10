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

#if !SAC_MOBILE
#include "api/sdl/MouseNativeTouchState.h"
#endif

#include "api/AssetAPI.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "api/StringInputAPI.h"
#include "api/JoystickAPI.h"

#include "base/EntityManager.h"
#include "base/PlacementHelper.h"
#include "base/Profiler.h"
#include "base/TouchInputManager.h"
#include "base/Common.h"
#include "base/TimeUtil.h"

#include "systems/ADSRSystem.h"
#include "systems/AnchorSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/AutoDestroySystem.h"
#include "systems/AutonomousAgentSystem.h"
#include "systems/BlinkSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/DebuggingSystem.h"
#include "systems/GraphSystem.h"
#include "systems/GridSystem.h"
#include "systems/BackInTimeSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/MusicSystem.h"
#include "systems/NetworkSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ScrollingSystem.h"
#include "systems/SpatialPartitionSystem.h"
#include "systems/SoundSystem.h"
#include "systems/SpotSystem.h"
#include "systems/TagSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/ZSQDSystem.h"
#include "systems/SwypeButtonSystem.h"

#include "systems/opengl/OpenGLTextureCreator.h"

#include "util/DataFileParser.h"
#include "util/Draw.h"
#include "util/Recorder.h"
#include "util/Random.h"
#include "util/LevelEditor.h"
#include "util/Tuning.h"

#if ! SAC_MOBILE
#include <SDL2/SDL_events.h>
#include <SDL2/SDL.h>
#endif

#if SAC_INGAME_EDITORS
#include "imgui.h"
#include "systems/opengl/OpenglHelper.h"
#include "stb_image.h"
#endif

Game::Game() {
#if SAC_INGAME_EDITORS
    gameType = GameType::Default;
#endif
    targetDT = 1.0f / 60.0f;

    isFinished = false;

#if !SAC_MOBILE
    mouseNativeTouchState = 0;
#endif

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
#if !DISABLE_AUTONOMOUS_SYSTEM
    AutonomousAgentSystem::CreateInstance();
#endif
#if !DISABLE_BLINK_SYSTEM
    BlinkSystem::CreateInstance();
#endif
    ButtonSystem::CreateInstance();
    CameraSystem::CreateInstance();
#if !DISABLE_COLLISION_SYSTEM
    CollisionSystem::CreateInstance();
#endif
#if !DISABLE_CONTAINER_SYSTEM
    ContainerSystem::CreateInstance();
#endif
#if !DISABLE_DEBUGGING_SYSTEM || SAC_DEBUG
    DebuggingSystem::CreateInstance();
#endif
#if !DISABLE_GRAPH_SYSTEM || SAC_DEBUG
    GraphSystem::CreateInstance();
#endif
#if !DISABLE_GRID_SYSTEM
    GridSystem::CreateInstance();
#endif
#if !DISABLE_MORPHING_SYSTEM
    MorphingSystem::CreateInstance();
#endif
    MusicSystem::CreateInstance();
    ParticuleSystem::CreateInstance();
    PhysicsSystem::CreateInstance();
    SpatialPartitionSystem::CreateInstance();
    RenderingSystem::CreateInstance();
#if !DISABLE_SCROLLING_SYSTEM
    ScrollingSystem::CreateInstance();
#endif
    SoundSystem::CreateInstance();
#if !DISABLE_SPOT_SYSTEM
    SpotSystem::CreateInstance();
    SpotBlockSystem::CreateInstance();
#endif
    TagSystem::CreateInstance();
    TextSystem::CreateInstance();
    TransformationSystem::CreateInstance();
#if !DISABLE_ZSQD_SYSTEM
    ZSQDSystem::CreateInstance();
#endif
#if !DISABLE_SWYPE_SYSTEM
    SwypeButtonSystem::CreateInstance();
#endif
    BackInTimeSystem::CreateInstance();

#if SAC_NETWORK
    NetworkSystem::CreateInstance();
#endif

    // default
    buildOrderedSystemsToUpdateList();

    fpsStats.reset(0);
    lastUpdateTime = TimeUtil::GetTime();
#if SAC_INGAME_EDITORS
    levelEditor = new LevelEditor(this);
    theRenderingSystem.editor = levelEditor;
#endif
}

bool Game::wantsAPI(ContextAPI::Enum api) const {
#if !SAC_NETWORK
    if (api == ContextAPI::Network) return false;
#endif

    return true;
}

void Game::buildOrderedSystemsToUpdateList() {
    orderedSystemsToUpdate.clear();

    #define ADD_IF_EXISTING(ptr) \
    PRAGMA_WARNING(warning(disable: 4127)) \
    do { \
        auto* _ptr = (ptr); \
        if (_ptr) orderedSystemsToUpdate.push_back(_ptr); \
    } while (false)

#if SAC_NETWORK
    ADD_IF_EXISTING(NetworkSystem::GetInstancePointer());
#endif
    ADD_IF_EXISTING(BackInTimeSystem::GetInstancePointer());

    ADD_IF_EXISTING(CameraSystem::GetInstancePointer());
    ADD_IF_EXISTING(ADSRSystem::GetInstancePointer());
    ADD_IF_EXISTING(AnimationSystem::GetInstancePointer());
    ADD_IF_EXISTING(AutoDestroySystem::GetInstancePointer());
#if !DISABLE_AUTONOMOUS_SYSTEM
    ADD_IF_EXISTING(AutonomousAgentSystem::GetInstancePointer());
#endif
#if !DISABLE_BLINK_SYSTEM
    ADD_IF_EXISTING(BlinkSystem::GetInstancePointer());
#endif
    ADD_IF_EXISTING(ButtonSystem::GetInstancePointer());
#if !DISABLE_CONTAINER_SYSTEM
    ADD_IF_EXISTING(ContainerSystem::GetInstancePointer());
#endif
#if !DISABLE_DEBUGGING_SYSTEM || SAC_DEBUG
    ADD_IF_EXISTING(DebuggingSystem::GetInstancePointer());
#endif
#if !DISABLE_GRAPH_SYSTEM || SAC_DEBUG
    ADD_IF_EXISTING(GraphSystem::GetInstancePointer());
#endif
#if !DISABLE_GRID_SYSTEM
    ADD_IF_EXISTING(GridSystem::GetInstancePointer());
#endif
#if !DISABLE_MORPHING_SYSTEM
    ADD_IF_EXISTING(MorphingSystem::GetInstancePointer());
#endif
    ADD_IF_EXISTING(MusicSystem::GetInstancePointer());
    ADD_IF_EXISTING(AnchorSystem::GetInstancePointer());
    ADD_IF_EXISTING(PhysicsSystem::GetInstancePointer());
    ADD_IF_EXISTING(SpatialPartitionSystem::GetInstancePointer());
#if !DISABLE_ZSQD_SYSTEM
    ADD_IF_EXISTING(ZSQDSystem::GetInstancePointer());
#endif
#if !DISABLE_COLLISION_SYSTEM
    ADD_IF_EXISTING(CollisionSystem::GetInstancePointer());
#if !DISABLE_ZSQD_SYSTEM
    ADD_IF_EXISTING(ParticuleSystem::GetInstancePointer());
    // 2nd pass
    ADD_IF_EXISTING(ZSQDSystem::GetInstancePointer());
#endif
#endif
#if !DISABLE_SCROLLING_SYSTEM
    ADD_IF_EXISTING(ScrollingSystem::GetInstancePointer());
#endif
    ADD_IF_EXISTING(SoundSystem::GetInstancePointer());
#if !DISABLE_SPOT_SYSTEM
    ADD_IF_EXISTING(SpotSystem::GetInstancePointer());
    ADD_IF_EXISTING(SpotBlockSystem::GetInstancePointer());
#endif
    ADD_IF_EXISTING(TagSystem::GetInstancePointer());
    ADD_IF_EXISTING(TextSystem::GetInstancePointer());
    ADD_IF_EXISTING(TransformationSystem::GetInstancePointer());
#if !DISABLE_SWYPE_SYSTEM
    ADD_IF_EXISTING(SwypeButtonSystem::GetInstancePointer());
#endif

#if SAC_ENABLE_LOG
    unusedSystems.clear();
    std::copy(orderedSystemsToUpdate.begin(), orderedSystemsToUpdate.end(), std::inserter(unusedSystems, unusedSystems.begin()));
#endif


    LOGV(1, orderedSystemsToUpdate.size() << " active systems");
}

Game::~Game() {
#if SAC_INGAME_EDITORS
    delete levelEditor;
#endif
    ADSRSystem::DestroyInstance();
    AnchorSystem::DestroyInstance();
    AnimationSystem::DestroyInstance();
    AutoDestroySystem::DestroyInstance();
#if !DISABLE_AUTONOMOUS_SYSTEM
    AutonomousAgentSystem::DestroyInstance();
#endif
#if !DISABLE_BLINK_SYSTEM
    BlinkSystem::DestroyInstance();
#endif
    ButtonSystem::DestroyInstance();
    CameraSystem::DestroyInstance();
#if !DISABLE_COLLISION_SYSTEM
    CollisionSystem::DestroyInstance();
#endif
#if !DISABLE_CONTAINER_SYSTEM
    ContainerSystem::DestroyInstance();
#endif
#if !DISABLE_DEBUGGING_SYSTEM || SAC_DEBUG
    DebuggingSystem::DestroyInstance();
#endif
#if !DISABLE_GRAPH_SYSTEM || SAC_DEBUG
    GraphSystem::DestroyInstance();
#endif
#if !DISABLE_GRID_SYSTEM
    GridSystem::DestroyInstance();
#endif
    BackInTimeSystem::DestroyInstance();
#if !DISABLE_MORPHING_SYSTEM
    MorphingSystem::DestroyInstance();
#endif
    MusicSystem::DestroyInstance();
    ParticuleSystem::DestroyInstance();
    PhysicsSystem::DestroyInstance();
    SpatialPartitionSystem::DestroyInstance();
    RenderingSystem::DestroyInstance();
#if !DISABLE_SCROLLING_SYSTEM
    ScrollingSystem::DestroyInstance();
#endif
    SoundSystem::DestroyInstance();
#if !DISABLE_SPOT_SYSTEM
    SpotSystem::DestroyInstance();
#endif
    TextSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
#if !DISABLE_ZSQD_SYSTEM
    ZSQDSystem::DestroyInstance();
#endif
#if !DISABLE_SWYPE_SYSTEM
    SwypeButtonSystem::DestroyInstance();
#endif

#if SAC_NETWORK
    NetworkSystem::DestroyInstance();
#endif
    orderedSystemsToUpdate.clear();

#if SAC_ENABLE_LOG
    unusedSystems.clear();
#endif

    EntityManager::DestroyInstance();
    Murmur::destroy();
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
#if ! SAC_MOBILE


    if (gameThreadContext->joystickAPI) {
        gameThreadContext->joystickAPI->update();
    }

    SDL_Event event;

    //if (! (SDL_GetAppState() & SDL_APPINPUTFOCUS)) {
        //LOGI("dont have the focus, dont treat inputs!");
        //return;
    //}

    while( SDL_PollEvent(&event) )
    {
        bool handled = false;
#if SAC_INGAME_EDITORS
        levelEditor->lock();
        levelEditor->addSDLEvent(event);
        levelEditor->unlock();
#endif

        //or try stringInputAPI
        if (!handled && wantsAPI(ContextAPI::StringInput)) {
            handled = gameThreadContext->stringInputAPI->eventSDL(&event);
        }
        //or try keyboardAPI
        if (!handled && gameThreadContext->keyboardInputHandlerAPI) {
            handled = gameThreadContext->keyboardInputHandlerAPI->eventSDL(&event);
        }
        //or try mouse
        if (!handled && mouseNativeTouchState) {
            handled = mouseNativeTouchState->eventSDL(&event);
        }
        //or try joystick
        if (!handled && gameThreadContext->joystickAPI) {
            handled = gameThreadContext->joystickAPI->eventSDL(&event);
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
                    }
                    break;
                }
                case SDL_WINDOWEVENT: {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        int w = event.window.data1;
                        int h = event.window.data2;
                        sac::setResolution(w, h);
                        changeResolution(w, h);
                    }
                    break;
                }
            }
        }
    }
#endif
}

void Game::loadFont(AssetAPI* asset, const char* name) {
    char tmp[1024];
    strncpy(tmp, name, 1024);
    strncat(tmp, ".font", 1024);
    FileBuffer file = asset->loadAsset(tmp);
    DataFileParser dfp;
    if (!dfp.load(file, tmp)) {
        LOGE("Invalid font description file: " << tmp);
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

    LOGV(1, "Loaded font: " << name << ". Found: " << h2wratio.size() << " entries");
    theTextSystem.registerFont(name, h2wratio);
}

void Game::changeResolution(int /*windowW*/, int /*windowH*/) {
    if (camera) {
        TRANSFORM(camera)->size = PlacementHelper::ScreenSize;
    }
}

void Game::sacInitFromRenderThread() {
#if SAC_ENABLE_PROFILING
    initProfiler();
#endif

    theRenderingSystem.init();

    // Auto-load all atlas
    {
        const std::string dpiFolder(OpenGLTextureCreator::DPI2Folder(OpenGLTextureCreator::dpi));
        std::list<std::string> atlas = renderThreadContext->assetAPI->listAssetContent(
            ".atlas", dpiFolder);
        LOGV(1, "Autoloading " << atlas.size() << " atlas");
        std::for_each(atlas.begin(), atlas.end(), [dpiFolder] (const std::string& a) -> void {
            theRenderingSystem.loadAtlas(dpiFolder + '/' + a);
        });
    }

    // Auto-load all fonts
    {
        std::list<std::string> fonts = renderThreadContext->assetAPI->listAssetContent(
            ".font");
        LOGV(1, "Autoloading " << fonts.size() << " fonts");
        std::for_each(fonts.begin(), fonts.end(), [this] (const std::string& typo) -> void {
            loadFont(renderThreadContext->assetAPI, typo.c_str());
        });

    }

#if SAC_INGAME_EDITORS
    ImGuiIO& io = ImGui::GetIO();

#if SAC_DESKTOP
    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_Tab] = ImGuiKey_Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
    io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
    io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
    io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
    io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
    io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
    io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
    io.KeyMap[ImGuiKey_Backspace] = ImGuiKey_Backspace;
    io.KeyMap[ImGuiKey_Enter] = ImGuiKey_Enter;
    io.KeyMap[ImGuiKey_Escape] = ImGuiKey_Escape;

    // only in SDL2
    // io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
    // io.GetClipboardTextFn = SDL_GetClipboardText;
#endif

    io.RenderDrawListsFn = RenderingSystem::ImImpl_RenderDrawLists;

    // Load font texture
    glGenTextures(1, &RenderingSystem::fontTex);
    glBindTexture(GL_TEXTURE_2D, RenderingSystem::fontTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    const void* png_data;
    unsigned int png_size;
    ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
    int tex_x, tex_y, tex_comp;
    void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    stbi_image_free(tex_data);
#endif
}

void Game::sacInitFromGameThread() {
#if SAC_INGAME_EDITORS
    levelEditor->init(gameThreadContext->keyboardInputHandlerAPI);
#endif

    LOGV(1, "Creating default camera");
    camera = theEntityManager.CreateEntity(HASH("camera", 0x526b9e0c));
    ADD_COMPONENT(camera, Camera);
    CAMERA(camera)->enable = true;
    CAMERA(camera)->clearColor = Color(0.5, 0.5, 0.5);
    ADD_COMPONENT(camera, Transformation);
    TRANSFORM(camera)->size = PlacementHelper::ScreenSize;
    theTouchInputManager.setCamera(camera);

    tuning.init(gameThreadContext->assetAPI);
    tuning.load("tuning.ini");

#if SAC_DEBUG
    glm::vec2 size = glm::vec2(TRANSFORM(camera)->size.x / FPS_DEBUG_N, 0);
    glm::vec2 bottomLeft = TRANSFORM(camera)->size * glm::vec2(-0.5f, -0.5f) - size.x  * 0.5f;

    for (int i=0; i<FPS_DEBUG_N; i++) {
        fpsStats.square[i] = theEntityManager.CreateEntity(HASH("__/square", 0x5b913efb));
        ADD_COMPONENT(fpsStats.square[i], Transformation);
        TRANSFORM(fpsStats.square[i])->size = size;
        TRANSFORM(fpsStats.square[i])->position = bottomLeft + size * (float)i;
        ADD_COMPONENT(fpsStats.square[i], Rendering);
        // RENDERING(fpsStats.square[i])->show = true;
        RENDERING(fpsStats.square[i])->color = Color::palette(i / (float)FPS_DEBUG_N, 0.5f);
        RENDERING(fpsStats.square[i])->flags = RenderingFlags::NonOpaque;

        fpsStats.text[i] = theEntityManager.CreateEntity(HASH("__/text", 0xba2ddac9));
        ADD_COMPONENT(fpsStats.text[i], Transformation);
        TRANSFORM(fpsStats.text[i])->position = glm::vec2(TRANSFORM(fpsStats.square[i])->position.x, (i % 2) * size.x * 2);
        ADD_COMPONENT(fpsStats.text[i], Text);
        TEXT(fpsStats.text[i])->text = "0.016";
        TEXT(fpsStats.text[i])->color = Color(0, 0, 0);
        // TEXT(fpsStats.text[i])->show = true;
        TEXT(fpsStats.text[i])->charHeight = 2 * size.x;
    }
#endif
}

int Game::saveState(uint8_t**) {
    return 0;
}

static float accumulator = 0.0f;
static float currentTime = 0.0f;
void Game::step() {
    PROFILE("Game", "step", BeginEvent);

    theRenderingSystem.waitDrawingComplete();

delta_time_computation:
    float newTime = TimeUtil::GetTime();
    float frameTime = newTime - currentTime;
    currentTime = newTime;

    theEntityManager.entityTemplateLibrary.update();

#if SAC_DESKTOP
    tuning.updateReload();
    theEntityManager.entityTemplateLibrary.updateReload();
#endif

#if SAC_ENABLE_PROFILING
    std::stringstream framename;
    framename << "update-" << (int)(delta * 1000000);
    PROFILE("Game", framename.str(), InstantEvent);
#endif

    accumulator += frameTime;
#if SAC_EMSCRIPTEN
    // targetDT = accumulator;
#endif

    /** dt calculation
     * rawFrameTime = raw calculated elpased time since last update
     * we want to always update with dt = 16ms
     * If rawFrameTime > 16, we do several successive updates
     * If < 16 it depends on the env:
     *    -> if multithreaded: we wait until it's >= 16
     *    -> if single-thread (emscripten): we assume it's 16, and keep the negative
     *       difference in the accumulator
     */

    #if SAC_INGAME_EDITORS
        bool doneOnce = false;
    #endif

    float dtFix = 0;
#if !SAC_BENCHMARK_MODE

    if (accumulator < targetDT) {
#if !SAC_EMSCRIPTEN
        TimeUtil::Wait(targetDT - accumulator);
        goto delta_time_computation;
#else
        dtFix = accumulator - targetDT;
        accumulator = targetDT;
#endif
    }

#if SAC_DEBUG
    {
        static int current = 0;
        char tmp[64];
        sprintf(tmp, "%d", (int)(accumulator * 1000));

        float height = accumulator / (5 * targetDT);
        TRANSFORM(fpsStats.square[current])->size.y = height;
        TRANSFORM(fpsStats.square[current])->position.y =
            TRANSFORM(camera)->size.y * (-0.5f) +
            height * 0.5f;
        TEXT(fpsStats.text[current])->text = tmp;
        current = (current + 1) % FPS_DEBUG_N;
        TEXT(fpsStats.text[current])->text = "...";
    }
#endif

    while (accumulator >= targetDT)
#endif

    {
        theTouchInputManager.Update();

        if (gameThreadContext->keyboardInputHandlerAPI) {
            gameThreadContext->keyboardInputHandlerAPI->update();
        }

        // update game state
    #if SAC_INGAME_EDITORS
        if (!doneOnce) {
            doneOnce = true;

            LevelEditor::lock();

            ImGuiIO& io = ImGui::GetIO();
            #if !SAC_MOBILE
            io.MouseWheel = theTouchInputManager.getWheel();
            #endif
            io.DeltaTime = targetDT;

            glm::vec2 p;
            #if SAC_MOBILE
            if (!theTouchInputManager.isTouched())
                io.MousePos =ImVec2(-1.0f, -1.0f);
            else
                p = theTouchInputManager.getTouchLastPositionScreen();
            #else
            p = theTouchInputManager.getOverLastPositionScreen();
            #endif

            p = glm::vec2(io.DisplaySize.x, io.DisplaySize.y) * (0.5f + p);
            p.y = io.DisplaySize.y - p.y;
            io.MousePos =ImVec2(p.x, p.y);
            io.MouseDown[0] = theTouchInputManager.isTouched(0);
            io.MouseDown[1] = theTouchInputManager.isTouched(1);
            ImGui::NewFrame();
            // Always tick levelEditor (manages AntTweakBar stuff)
            levelEditor->tick(targetDT);

            LevelEditor::unlock();
        }
    #endif

        Draw::Update();

#if SAC_INGAME_EDITORS
        if (gameType == GameType::Default || gameType == GameType::SingleStep ) {
#endif

#if SAC_ENABLE_LOG
            static int timer = 0;
            //every 60 secs, log it
            if (++timer == 60*60) {
                timer = 0;
                for (auto* sys : unusedSystems) {
                    LOGW("System " << INV_HASH(sys->getId()) << " has (yet) not been used");
                }
            }
#endif

            LOGV(3, "Update systems");

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
                sys->Update(targetDT);
            }

#if SAC_INGAME_EDITORS
            if (gameType == GameType::SingleStep)
                gameType = GameType::LevelEditor;
        }
#endif

        #if SAC_INGAME_EDITORS
        static float speedFactor = 1.0f;
        switch (gameType) {
            case GameType::LevelEditor:
                break;
            case GameType::SingleStep:
                LOGI("Single stepping the game (delta: " << targetDT << " ms)");
                tick(targetDT);
                break;
            case GameType::Replay:
                break;
            default:
                tick(targetDT * speedFactor);
        }

    #else
        LOGV(3, "Update game");
        tick(targetDT);
    #endif

        accumulator -= targetDT;
    }

    accumulator += dtFix;

    LOGV(3, "Produce rendering frame");
    // produce 1 new frame
#if SAC_INGAME_EDITORS
    if (gameType == GameType::Replay) {
        int count = 0;
        auto* commands = levelEditor->getFrame(&count);;
        theRenderingSystem.forceRenderCommands(commands, count);
        ImGui::Render();
    } else {
        theRenderingSystem.Update(frameTime);
    }
#else
    theRenderingSystem.Update(frameTime);
#endif

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
        if (count == 3000) {
            LOGI("FPS avg/min/max : " <<
                (300.0 / (t - fpsStats.since)) << '/' << (1.0 / fpsStats.maxDt) << '/' << (1.0 / fpsStats.minDt));
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

void Game::preDestroy() {
    Draw::ClearAll();

    /* Delete before destroying game, as system will be destroyed */
    theEntityManager.deleteAllEntities();
}
