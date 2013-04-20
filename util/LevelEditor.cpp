#if SAC_INGAME_EDITORS

#include "LevelEditor.h"
#include "IntersectionUtil.h"
#include "../base/EntityManager.h"
#include "../base/TouchInputManager.h"
#include "../base/PlacementHelper.h"
#include "../systems/TransformationSystem.h"
#include "../systems/RenderingSystem.h"
#include "../systems/CameraSystem.h"
#include <GL/glfw.h>
#include <AntTweakBar.h>
#include <mutex>
#include <set>
#include <glm/gtx/rotate_vector.hpp>

#include "DebugConsole.h"

namespace EditorMode {
    enum Enum {
        Selection,
        Gallery
    };
}

std::mutex twMutex;
static void _lock() {
	twMutex.lock();
}

static void _unlock() {
    twMutex.unlock();
}

static int _TwEventMouseButtonGLFW(int glfwButton, int glfwAction) {
    _lock();
    int rValue = TwEventMouseButtonGLFW(glfwButton, glfwAction);
    _unlock();

    return rValue;
}
static int _TwEventMousePosGLFW(int mouseX, int mouseY) {
    _lock();
    int rValue = TwEventMousePosGLFW(mouseX, mouseY);
    _unlock();

    return rValue;
}
static int _TwEventMouseWheelGLFW(int pos) {
    _lock();
    int rValue = TwEventMouseWheelGLFW(pos);
    _unlock();

    return rValue;
}
static int _TwEventKeyGLFW(int glfwKey, int glfwAction) {
    _lock();
    int rValue = TwEventKeyGLFW(glfwKey, glfwAction);
    _unlock();

    return rValue;
}
static int _TwEventCharGLFW(int glfwChar, int glfwAction) {
    _lock();
    int rValue = TwEventCharGLFW(glfwChar, glfwAction);
    _unlock();

    return rValue;
}

void LevelEditor::lock() {
    _lock();
}

void LevelEditor::unlock() {
    _unlock();
}

struct LevelEditor::LevelEditorDatas {
    EditorMode::Enum mode;

    Entity over;
    Entity selected, gallerySelected;
    Entity selectionDisplay, overDisplay;
    Color originalColor;
    float selectionColorChangeSpeed;

    glm::vec2 lastMouseOverPosition;
    glm::vec2 selectedOriginalPos;

    unsigned activeCameraIndex;
    bool spaceWasPressed;

    Entity gallery;
    std::vector<Entity> galleryItems;
    std::set<std::string> logControlFiles;

    void changeMode(EditorMode::Enum newMode);
    void buildGallery();
    void destroyGallery();

    void updateModeSelection(float dt, const glm::vec2& mouseWorldPos, int wheelDiff);
    void updateModeGallery(float dt, const glm::vec2& mouseWorldPos, int wheelDiff);

    void select(Entity e);
    void deselect(Entity e);
};

static TwBar* createTweakBarForEntity(Entity e, const std::string& barName) {
    TwBar* bar = TwNewBar(barName.c_str());
    std::vector<std::string> systems = ComponentSystem::registeredSystemNames();
    for (unsigned i=0; i<systems.size(); i++) {
        ComponentSystem* system = ComponentSystem::Named(systems[i]);
        system->addEntityPropertiesToBar(e, bar);
        std::stringstream fold;
        fold << TwGetBarName(bar) << '/' << systems[i] << " opened=false";
        TwDefine(fold.str().c_str());
    }
    return bar;
}

static void showTweakBarForEntity(Entity e) {
    std::stringstream barName;

#ifdef SAC_DEBUG
    barName << theEntityManager.entityName(e);
#else
    barName << e;
#endif

    TwBar* bar = TwGetBarByName(barName.str().c_str());
    if (bar == 0) {
        bar = createTweakBarForEntity(e, barName.str());
        barName << " alpha=190 refresh=0,016";
        TwDefine (barName.str().c_str());
    } else {
        barName << " visible=true iconified=false";
        TwDefine(barName.str().c_str());
    }
    TwDefine(" GLOBAL iconpos=bottomright");
}

static void buttonCallback(void* e) {
    showTweakBarForEntity((Entity)(e));
}

void LevelEditor::LevelEditorDatas::select(Entity e) {
    _lock();
    showTweakBarForEntity(e);
    _unlock();
    TRANSFORM(selectionDisplay)->parent = e;
    TRANSFORM(selectionDisplay)->size = TRANSFORM(e)->size + glm::vec2(0.1f);
    RENDERING(selectionDisplay)->show = true;
    originalColor = RENDERING(e)->color;
    // RENDERING(selectionDisplay)->color = Color(1, 0, 0, 0.7);
}

void LevelEditor::LevelEditorDatas::deselect(Entity) {
    RENDERING(selectionDisplay)->show = false;
}

TwBar* entityListBar, *debugConsoleBar, *logBar;
LevelEditor::LevelEditor() {
    datas = new LevelEditorDatas();
    datas->activeCameraIndex = 0;
    datas->mode = EditorMode::Selection;
    datas->selectionColorChangeSpeed = -0.5;
    datas->selectionDisplay = theEntityManager.CreateEntity("debug_selection");
    ADD_COMPONENT(datas->selectionDisplay, Rendering);
    ADD_COMPONENT(datas->selectionDisplay, Transformation);
    TRANSFORM(datas->selectionDisplay)->z = -0.001;
    RENDERING(datas->selectionDisplay)->color = Color(1, 0, 0, 0.7);

    datas->overDisplay = theEntityManager.CreateEntity("debug_over");
    ADD_COMPONENT(datas->overDisplay, Rendering);
    ADD_COMPONENT(datas->overDisplay, Transformation);
    TRANSFORM(datas->overDisplay)->z = -0.001;
    RENDERING(datas->overDisplay)->color = Color(0, 0, 1, 0.7);

    TwInit(TW_OPENGL, NULL);

    glfwSetMouseButtonCallback((GLFWmousebuttonfun) _TwEventMouseButtonGLFW);
    glfwSetMousePosCallback((GLFWmouseposfun) _TwEventMousePosGLFW);
    glfwSetMouseWheelCallback((GLFWmousewheelfun) _TwEventMouseWheelGLFW);
    glfwSetKeyCallback((GLFWkeyfun) _TwEventKeyGLFW);
    glfwSetCharCallback((GLFWcharfun) _TwEventCharGLFW);

    debugConsoleBar = TwNewBar("Debug_Console");
    logBar = TwNewBar("Log_Control");
    entityListBar = TwNewBar("EntityList");
    TwDefine(" Debug_Console iconified=true ");
    TwDefine(" Log_Control iconified=true ");
    TwDefine(" EntityList iconified=true ");
    // add default choice for log control
    TwEnumVal modes[] = {
        {LogVerbosity::FATAL, "Fatal"},
        {LogVerbosity::ERROR, "Error"},
        {LogVerbosity::WARNING, "Warning"},
        {LogVerbosity::INFO, "Info"},
        {(int)LogVerbosity::INFO + 1, "Verbose1"},
        {(int)LogVerbosity::INFO + 2, "Verbose2"},
        {(int)LogVerbosity::INFO + 3, "Verbose3"}
    };
    TwType type = TwDefineEnum("Verbosity", modes, 7);
    TwAddVarRW(logBar, "Verbosity", type, &logLevel, "");
    TwAddSeparator(logBar, "File control", "");
}

LevelEditor::~LevelEditor() {
    delete datas;
}

static void TW_CALL LogControlSetCallback(const void *value, void *clientData) {
    const bool* l = static_cast<const bool*>(value);
    const char* s = static_cast<const char*>(clientData);
    LOGI("Change verbosity for '" << s << "' -> '" << *l << "'")
    verboseFilenameFilters[s] = *l;
}

static void TW_CALL LogControlGetCallback(void *value, void *clientData) {
    bool* l = static_cast<bool*>(value);
    const char* s = static_cast<const char*>(clientData);
    *l = verboseFilenameFilters[s];
}

void LevelEditor::tick(float dt) {
    // update entity list every sec
    static float accum = 1;
    accum += dt;
    if (accum > 1) {
        lock();

        std::vector<Entity> entities = theEntityManager.allEntities();
        TwRemoveAllVars(entityListBar);
        for (unsigned i=0; i<entities.size(); i++) {
            if (entities[i] == datas->selectionDisplay || entities[i] == datas->overDisplay)
                continue;
            std::stringstream n;
#ifdef SAC_DEBUG
            n << theEntityManager.entityName(entities[i]);
#else
            n << entities[i];
#endif

            if (n.str().find("__debug") == 0 || n.str().find("__text") == 0)
                continue;

            TwAddButton(entityListBar, n.str().c_str(), (TwButtonCallback)&buttonCallback, (void*)entities[i], "");
        }
        accum = 0;

        // update log buttons too
        for (auto it = verboseFilenameFilters.begin(); it!=verboseFilenameFilters.end(); ++it) {
            if (datas->logControlFiles.find(it->first) == datas->logControlFiles.end()) {
                TwAddVarCB(logBar, it->first.c_str(), TW_TYPE_BOOLCPP, LogControlSetCallback, LogControlGetCallback, strdup(it->first.c_str()), "");
                datas->logControlFiles.insert(it->first);
            }
        }
        unlock();
    }
    std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    Entity camera = 0;
    for (unsigned i=0; i<cameras.size(); i++) {
        if (CAMERA(cameras[i])->fb == DefaultFrameBufferRef) {
            camera = cameras[i];
            break;
        }
    }
    if (!camera) {
        LOGE("No active camera")
        return;
    }

    int x, y;
    glfwGetMousePos(&x, &y);
    glm::vec2 windowPos(x / (float)PlacementHelper::WindowWidth - 0.5f, 0.5f - y / (float)PlacementHelper::WindowHeight);

    const glm::vec2 position = TRANSFORM(camera)->worldPosition + glm::rotate(windowPos * TRANSFORM(camera)->size, TRANSFORM(camera)->worldRotation);

    static int prevWheel = 0;
    int wheel = glfwGetMouseWheel();
    int wheelDiff = wheel - prevWheel;
    prevWheel = wheel;

    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE &&
        glfwGetMouseButton(GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
        datas->lastMouseOverPosition = position;
    }

    Color& selectedColor = RENDERING(datas->selectionDisplay)->color;
    selectedColor.a += datas->selectionColorChangeSpeed * dt;
    if (selectedColor.a < 0.5) {
        selectedColor.a = 0.5;
        datas->selectionColorChangeSpeed *= -1;
    } else if (selectedColor.a > 1) {
        selectedColor.a = 1;
        datas->selectionColorChangeSpeed *= -1;
    }
    RENDERING(datas->overDisplay)->color.a = selectedColor.a;


    switch (datas->mode) {
        case EditorMode::Selection:
            datas->updateModeSelection(dt, position, wheelDiff);
            break;
        case EditorMode::Gallery:
            datas->updateModeGallery(dt, position, wheelDiff);
            break;
    }
}

void LevelEditor::LevelEditorDatas::changeMode(EditorMode::Enum newMode) {
    if (newMode == mode)
        return;

    switch (newMode) {
        case EditorMode::Gallery:
            std::cout << "GalleryMode" << std::endl;
            buildGallery();
            break;
        default:
            std::cout << "SelectionMode" << std::endl;
            destroyGallery();
    }
    mode = newMode;
}

void LevelEditor::LevelEditorDatas::buildGallery() {
#if 0
    int textureCount = theRenderingSystem.assetTextures.size();
    float width = theRenderingSystem.cameras[activeCameraIndex].worldSize.X;
    const int elementPerRow = 7;
    float spacing = 0;
    float texSize = width / (elementPerRow + (elementPerRow -1) * spacing);
    glm::vec2 gallerySize = Vector2(width, (textureCount / elementPerRow) * (1.0f * texSize));;

    gallery = theEntityManager.CreateEntity();
    ADD_COMPONENT(gallery, Transformation);
    TRANSFORM(gallery)->position = theRenderingSystem.cameras[activeCameraIndex].worldPosition;
    TRANSFORM(gallery)->z = 0.99;
    TRANSFORM(gallery)->size = gallerySize;
    ADD_COMPONENT(gallery, Rendering);
    RENDERING(gallery)->show = true;
    RENDERING(gallery)->color = Color(0, 0, 0, 0.8);

    int count = 0;
    for (std::map<std::string, TextureRef>::iterator it=theRenderingSystem.assetTextures.begin();
        it!=theRenderingSystem.assetTextures.end();
        ++it, count++) {
        int column = count % elementPerRow, row = count / elementPerRow;
        Entity e = theEntityManager.CreateEntity();
        ADD_COMPONENT(e, Transformation);
        TRANSFORM(e)->z = 0.001;
        TRANSFORM(e)->parent = gallery;
        const glm::vec2& te = theRenderingSystem.getTextureSize(it->first);
        if (te.X >= te.Y)
            TRANSFORM(e)->size = glm::vec2(texSize, texSize * te.Y / te.X);
        else
            TRANSFORM(e)->size = glm::vec2(texSize * te.X / te.Y, texSize);
        TRANSFORM(e)->position = glm::vec2(column * texSize * 1.0f, row * texSize * 1.0f) - gallerySize * 0.5f + Vector2(texSize * 0.5f);
        ADD_COMPONENT(e, Rendering);
        RENDERING(e)->show = true;
        RENDERING(e)->texture = it->second;
        RENDERING(e)->effectRef = theRenderingSystem.loadEffectFile("over.fs");
        // RENDERING(e)->opaqueType = RenderingComponent::FULL_OPAQUE;
        galleryItems.push_back(e);
    }
    gallerySelected = galleryItems.front();
#endif
}

void LevelEditor::LevelEditorDatas::destroyGallery() {
    for(unsigned i=0; i<galleryItems.size(); i++) {
        theEntityManager.DeleteEntity(galleryItems[i]);
    }
    galleryItems.clear();
    theEntityManager.DeleteEntity(gallery);
}

void LevelEditor::LevelEditorDatas::updateModeSelection(float /*dt*/, const glm::vec2& /*mouseWorldPos*/, int /*wheelDiff*/) {
#if 0
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
        if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
            if (selected)
                selectedOriginalPos = TRANSFORM(selected)->position;
            over = 0;

            std::vector<Entity> entities = theRenderingSystem.RetrieveAllEntityWithComponent();
            float nearest = 10000;
            for (unsigned i=0; i<entities.size(); i++) {
                if (entities[i] == overDisplay || entities[i] == selectionDisplay)
                    continue;
                if (entities[i] == selected)
                    continue;
                if (!RENDERING(entities[i])->show)
                    continue;
                if (IntersectionUtil::pointRectangle(mouseWorldPos, TRANSFORM(entities[i])->worldPosition, TRANSFORM(entities[i])->size)) {
                    float d = glm::distance(mouseWorldPos, TRANSFORM(entities[i])->worldPosition);
                    if (d < nearest) {
                        over = entities[i];
                        nearest = d;
                    }
                }
            }

            if (over) {
                TRANSFORM(overDisplay)->parent = over;
                TRANSFORM(overDisplay)->size = TRANSFORM(over)->size + glm::vec2(0.1f);
                RENDERING(overDisplay)->show = true;
            } else {
                RENDERING(overDisplay)->show = false;
            }
        } else {
            if (over) {
                if (selected)
                    deselect(selected);
                selected = over;
                LOGI("Selected entity: '" << theEntityManager.entityName(selected) << "'")
                select(selected);
                over = 0;
                RENDERING(overDisplay)->show = false;
            }
        }
    }
#if 0
    else {
        if (selected) {
            TRANSFORM(selected)->position = selectedOriginalPos + mouseWorldPos - lastMouseOverPosition;
        }
    }

    if (selected) {
        if (wheelDiff) {
            bool shift = glfwGetKey( GLFW_KEY_LSHIFT );
            bool ctrl = glfwGetKey( GLFW_KEY_LCTRL );

            if (!shift && !ctrl) {
                TRANSFORM(selected)->rotation += 2 * wheelDiff * dt;
            } else {
                if (shift) {
                    TRANSFORM(selected)->size.X *= (1 + 1 * wheelDiff * dt);
                }
                if (ctrl) {
                    TRANSFORM(selected)->size.Y *= (1 + 1 * wheelDiff * dt);
                }
            }
        }
    }
#endif
#if 0
    // camera movement
    {
        RenderingSystem::Camera& camera = theRenderingSystem.cameras[activeCameraIndex];
        float moveSpeed = glfwGetKey(GLFW_KEY_LSHIFT) ? 1 : 0.25;
        if (glfwGetKey(GLFW_KEY_LEFT)) {
            camera.worldPosition.X -= moveSpeed * camera.worldSize.X * dt;
        } else if (glfwGetKey(GLFW_KEY_RIGHT)) {
            camera.worldPosition.X += moveSpeed * camera.worldSize.X * dt;
        }
        if (glfwGetKey(GLFW_KEY_DOWN)) {
            camera.worldPosition.Y -= moveSpeed * camera.worldSize.Y  * dt;
        } else if (glfwGetKey(GLFW_KEY_UP)) {
            camera.worldPosition.Y += moveSpeed * camera.worldSize.Y * dt;
        }
        float zoomSpeed = glfwGetKey(GLFW_KEY_LSHIFT) ? 2 : 1.1;
        if (glfwGetKey(GLFW_KEY_KP_ADD)) {
            camera.worldSize *= 1 - zoomSpeed * dt;
        } else if (glfwGetKey(GLFW_KEY_KP_SUBTRACT)) {
            camera.worldSize *= 1 + zoomSpeed * dt;
        }
    }
    // camera switching
    {
        for (unsigned i=0; i<theRenderingSystem.cameras.size(); i++) {
            if (glfwGetKey(GLFW_KEY_KP_1 + i) && i != activeCameraIndex) {
                std::cout << "new active cam: " << i << std::endl;
                theRenderingSystem.cameras[activeCameraIndex].enable = false;
                activeCameraIndex = i;
                theRenderingSystem.cameras[activeCameraIndex].enable = true;
                break;
            }
        }
    }
    // adding entities
    if (glfwGetKey(GLFW_KEY_SPACE)) {
        spaceWasPressed = true;
    } else if (spaceWasPressed) {
        spaceWasPressed = false;
        if (!selected) {
            // Add new entity
            Entity e = theEntityManager.CreateEntity();
            ADD_COMPONENT(e, Transformation);
            TRANSFORM(e)->position = theRenderingSystem.cameras[activeCameraIndex].worldPosition;
            TRANSFORM(e)->z = 0.95;
            ADD_COMPONENT(e, Rendering);
            RENDERING(e)->show = true;
            selected = e;
        } else
            changeMode(EditorMode::Gallery);
    }
#endif
#endif
}

#if 1
void LevelEditor::LevelEditorDatas::updateModeGallery(float, const glm::vec2&, int) {
#else
void LevelEditor::LevelEditorDatas::updateModeGallery(float dt, const glm::vec2& mouseWorldPos, int wheelDiff) {
#endif
#if 0
    if (glfwGetKey(GLFW_KEY_SPACE)) {
        spaceWasPressed = true;
    } else if (spaceWasPressed) {
        spaceWasPressed = false;
        changeMode(EditorMode::Selection);
    }

    if (wheelDiff) {
        float speed = wheelDiff * theRenderingSystem.cameras[activeCameraIndex].worldSize.y * ( glfwGetKey( GLFW_KEY_LSHIFT ) ? 2 : 0.8);
        TRANSFORM(gallery)->position.y -= speed * dt;
    }

    for (unsigned i=0; i<galleryItems.size(); i++) {
        if (IntersectionUtil::pointRectangle(mouseWorldPos, TRANSFORM(galleryItems[i])->worldPosition, TRANSFORM(galleryItems[i])->size)) {
            if (galleryItems[i] == gallerySelected)
                break;
            RENDERING(gallerySelected)->effectRef = theRenderingSystem.loadEffectFile("over.fs");
            gallerySelected = galleryItems[i];
            RENDERING(gallerySelected)->effectRef = theRenderingSystem.loadEffectFile("selected.fs");
            break;
        }
    }

    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        if (selected) {
            RENDERING(selected)->texture = RENDERING(gallerySelected)->texture;
        }
        changeMode(EditorMode::Selection);
    }
#endif
}
#endif
