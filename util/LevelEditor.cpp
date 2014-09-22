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



#if SAC_INGAME_EDITORS

#include "LevelEditor.h"
#include "IntersectionUtil.h"
#include "base/TouchInputManager.h"
#include <base/EntityManager.h>
#include <systems/TransformationSystem.h>
#include <systems/RenderingSystem.h>
#include <systems/CameraSystem.h>
#include <systems/TextSystem.h>
#include "base/PlacementHelper.h"

#include <mutex>
#include <set>
#include <glm/gtx/rotate_vector.hpp>

#include "DebugConsole.h"
#if SAC_NETWORK
#include "../systems/NetworkSystem.h"
#endif

#if SAC_INGAME_EDITORS
int LevelEditor::DebugAreaWidth =
#if SAC_DESKTOP
    600;
#else
    0;
#endif
int LevelEditor::DebugAreaHeight =
#if SAC_DESKTOP
    200;
#else
    0;
#endif
#endif

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

static std::string entityToName(Entity e) {
    std::stringstream s;
    s << theEntityManager.entityName(e) << ' ' << (e & 0xf7ffffff);
    return s.str();
}
#if 0

//see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twcopystdstringtoclientfunc
static void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
  // Copy the content of souceString handled by the AntTweakBar library to destinationClientString handled by your application
  destinationClientString = sourceLibraryString;
}
#endif

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
    Color originalColor;
    float selectionColorChangeSpeed;

    glm::vec2 lastMouseOverPosition;
    glm::vec2 selectedOriginalPos;

    unsigned activeCameraIndex;
    bool spaceWasPressed;

    Entity gallery;
    std::vector<Entity> galleryItems;

    std::map<Entity, std::string> barVar;

#if SAC_ENABLE_LOG
    std::set<std::string> logControlFiles;
#endif

    void changeMode(EditorMode::Enum newMode);
    void buildGallery();
    void destroyGallery();

    void updateModeSelection(float dt, const glm::vec2& mouseWorldPos, int wheelDiff);
    void updateModeGallery(float dt, const glm::vec2& mouseWorldPos, int wheelDiff);

    void select(Entity e);
    void deselect(Entity e);
};

static void createTweakBarForEntity(Entity e, const std::string& barName) {
    std::vector<hash_t> systems = ComponentSystem::registeredSystemIds();
    for (unsigned i=0; i<systems.size(); i++) {
        ComponentSystem* system = ComponentSystem::GetById(systems[i]);
        system->addEntityPropertiesToBar(e, 0);
    }
}

#if 0
static void showTweakBarForEntity(Entity e) {
    std::stringstream barName;

    barName << entityToTwName(e);

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
    uint64_t ptr = reinterpret_cast<uint64_t>(e);
    showTweakBarForEntity((Entity)ptr);
}
TwBar* entityListBar, *debugConsoleBar, *logBar, *dumpEntities;
#endif

void LevelEditor::LevelEditorDatas::select(Entity e) {
#if 0
    _lock();
    showTweakBarForEntity(e);
    _unlock();
#endif
}

void LevelEditor::LevelEditorDatas::deselect(Entity) {

}



namespace EntityListMode {
    enum Enum {
        All = 0,
        VisibleOnly,
        UnderMouse
    };
}
EntityListMode::Enum listMode = EntityListMode::All;

LevelEditor::LevelEditor() {
    datas = new LevelEditorDatas();
    datas->activeCameraIndex = 0;
    datas->mode = EditorMode::Selection;
    datas->selectionColorChangeSpeed = -0.5;

#if 0
    TwInit(TW_OPENGL, NULL);
    TwDefine(" GLOBAL fontsize=3 "); // use large font

    //to copy std string
    TwCopyStdStringToClientFunc(CopyStdStringToClient);

    DebugConsole::Instance().initTW();

    entityListBar = TwNewBar("EntityList");
    {
        // add modes to entity list
        TwEnumVal modes[] = {
            {EntityListMode::All, "All"},
            {EntityListMode::VisibleOnly, "Only Visible"},
            {EntityListMode::UnderMouse, "Under Mouse"}
        };
        TwType type = TwDefineEnum("Mode", modes, sizeof(modes)/sizeof(TwEnumVal));
        TwAddVarRW(entityListBar, "Mode", type, &listMode, "");
        TwAddSeparator(entityListBar, "--- Entities ---", "");
    }

    dumpEntities = TwNewBar("DumpEntities");

    TwDefine(" EntityList iconified=true ");
    TwDefine(" DumpEntities iconified=true ");

#if SAC_ENABLE_LOG
    logBar = TwNewBar("Log_Control");
    TwDefine(" Log_Control iconified=true ");
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
    TwType type = TwDefineEnum("Verbosity", modes, sizeof(modes)/sizeof(TwEnumVal));
    TwAddVarRW(logBar, "Verbosity", type, &logLevel, "");
    TwAddSeparator(logBar, "File control", "");
#endif
#endif

}

LevelEditor::~LevelEditor() {
    delete datas;
}

#if SAC_ENABLE_LOG
#if 0
static void TW_CALL LogControlSetCallback(const void *value, void *clientData) {
    const bool* l = static_cast<const bool*>(value);
    const char* s = static_cast<const char*>(clientData);
    LOGI("Change verbosity for '" << s << "' -> '" << *l << "'");
    verboseFilenameFilters[s] = *l;
}

static void TW_CALL LogControlGetCallback(void *value, void *clientData) {
    bool* l = static_cast<bool*>(value);
    const char* s = static_cast<const char*>(clientData);
    *l = verboseFilenameFilters[s];
}
#endif
#endif

static void DumpSystemEntities(void *clientData) {
    ComponentSystem* s = ComponentSystem::GetById(*((hash_t*) clientData));

    LOGW((char*) clientData << " system dump");
    LOGW("##########################################################");
    s->forEachEntityDo([] (Entity e) -> void {
        LOGW("   " << theEntityManager.entityName(e));
    });
    LOGW("##########################################################");
}

void LevelEditor::init() {
    ImGui::SetNewWindowDefaultPos(ImVec2(ImGui::GetIO().DisplaySize.x - DebugAreaWidth, 0));
}

static std::string displayGroup(Entity e) {
    std::string name (theEntityManager.entityName(e));

    //if name contains a slash ('/'), then first part is the group's name
    auto slashPos = name.find('/');
    if (slashPos != std::string::npos) name = name.substr(0, slashPos);

    if (name.find("__debug") == 0) name = "__debug";
    if (name.find("__text") == 0) name = "__text";


    return name;
}

std::map<Entity, bool> showEntityWindow;

static void imguiInputFilter() {
    ImVec2 pos, end;
    pos = end = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    end.x += size.x;
    end.y += size.y;
    if (ImGui::IsMouseHoveringBox(
        ImGui::GetWindowPos(),
        end)) {
        // force no click state
        theTouchInputManager.resetState();
    }
}

void LevelEditor::tick(float dt) {
    // build entity-list Window
    std::vector<Entity> entities = theEntityManager.allEntities();
    if (!ImGui::Begin("Entity List")) {
        ImGui::End();
        return;
    }

    // CollapsingHeader
    for (unsigned i=0; i<entities.size(); i++) {
        Entity e = entities[i];

        std::stringstream n;
        n << entityToName(e);

        if (ImGui::Button(n.str().c_str())) {
            bool keepOpen = true;
            showEntityWindow[e] = true;
        }

        if (ImGui::IsHovered()) {
            auto* rc = theRenderingSystem.Get(e, false);
            if (rc) rc->highLight = true;
            auto* tc = theTextSystem.Get(e, false);
            if (tc) tc->highLight = true;
        }

    }

    imguiInputFilter();
    ImGui::End();

    for (auto& p: showEntityWindow) {
        if (p.second) {
            Entity e = p.first;
            ImGui::Begin(entityToName(e).c_str(), &p.second);

            createTweakBarForEntity(e, "");

            imguiInputFilter();
            ImGui::End();
        }
    }

    #if 0
    // update entity list every sec
    static float accum = 1;

    int barVisible = 0;
    TwGetParam(entityListBar, NULL, "visible", TW_PARAM_INT32, 1, &barVisible);
    if (!barVisible)
        return;

    accum += dt;
    if (accum > 1) {
        lock();

        std::vector<Entity> entities = theEntityManager.allEntities();
        auto newEnd = entities.end();

        // Filter based on type
        switch (listMode) {
            case EntityListMode::All:
                break;
            case EntityListMode::VisibleOnly: {
                std::vector<TransformationComponent*> cameras;
                cameras.resize(theCameraSystem.entityCount());
                // get cameras
                theCameraSystem.forEachECDo([&cameras] (Entity e, CameraComponent* cc) -> void {
                    cameras[cc->id] = TRANSFORM(e);
                });
                // filter invisible entities
                newEnd = std::remove_if(entities.begin(), newEnd, [this, &cameras] (Entity e) -> bool {
                    const auto* rc = theRenderingSystem.Get(e, false);
                    const auto* txtc = theTextSystem.Get(e, false);
                    if (!rc && !txtc)
                        return true;
                    if ((rc && !rc->show) && (txtc && !txtc->show))
                        return true;
                    const auto* tc = theTransformationSystem.Get(e, false);
                    if (!tc)
                        return true;
                    for (unsigned i=0; i<cameras.size(); i++) {
                        if (rc) {
                            if ((rc->cameraBitMask & (1 << i)) && IntersectionUtil::rectangleRectangle(TRANSFORM(e), cameras[i]))
                                return false;
                        }
                        else {
                            if ((txtc->cameraBitMask & (1 << i)) && IntersectionUtil::rectangleRectangle(TRANSFORM(e), cameras[i]))
                                return false;
                        }
                    }
                    return true;
                });
                break;
            }
            case EntityListMode::UnderMouse: {
                LOGT_EVERY_N(10, "Under-mouse entity filtering");
                break;
            }

        }
        entities.resize(newEnd - entities.begin());
        const auto existing = entities;

        // Filter out entities already in bar
        newEnd = std::remove_if(entities.begin(), newEnd, [this] (Entity e) -> bool {
            return datas->barVar.find(e) != datas->barVar.end();
        });
        entities.resize(newEnd - entities.begin());

        // Build entity groups
        std::map<std::string, std::vector<Entity> > groups;
        for (unsigned i=0; i<entities.size(); i++) {
            groups[displayGroup(entities[i])].push_back(entities[i]);
        }

        // Add missing entities to bar
        for (unsigned i=0; i<entities.size(); i++) {
            Entity e = entities[i];

            std::stringstream n;
            n << entityToTwName(e);

            std::string define = "";
            if (groups[displayGroup(e)].size() > 1) {
                define = "group='" + displayGroup(e) + "'";
            }
            uint64_t ptr = entities[i];
            TwAddButton(entityListBar, n.str().c_str(), (TwButtonCallback)&buttonCallback, (void*)ptr, define.c_str());
            bool added = datas->barVar.insert(std::make_pair(e, n.str())).second;
            LOGF_IF(!added, "Added is false: " << e << '/' << n.str());

        }
        // Make sure groups are there too
        for (const auto gp: groups) {
            if (gp.second.size() > 1) {
                const std::string d("EntityList/'" + gp.first + "' opened=false");
                TwDefine(d.c_str());
            }
        }

        // Remove deleted/filtered entities from bar
        for (auto it=datas->barVar.begin(); it!=datas->barVar.end(); ) {
            if (std::find(existing.begin(), existing.end(), it->first) == existing.end()) {
                TwRemoveVar(entityListBar, it->second.c_str());
                datas->barVar.erase(it++);
            } else {
                ++it;
            }
        }

        LOGF_IF(datas->barVar.size() != existing.size(), "Incoherent var count: " << datas->barVar.size() << " != " << existing.size());
        accum = 0;

#if SAC_ENABLE_LOG
        // update log buttons too
        for (auto it = verboseFilenameFilters.begin(); it!=verboseFilenameFilters.end(); ++it) {
            if (datas->logControlFiles.find(it->first) == datas->logControlFiles.end()) {
                TwAddVarCB(logBar, it->first.c_str(), TW_TYPE_BOOLCPP, LogControlSetCallback, LogControlGetCallback, strdup(it->first.c_str()), "");
                datas->logControlFiles.insert(it->first);
            }
        }
#endif
        unlock();
    }
    #endif
}

void LevelEditor::LevelEditorDatas::changeMode(EditorMode::Enum newMode) {
    if (newMode == mode)
        return;

    switch (newMode) {
        case EditorMode::Gallery:
            LOGI("GalleryMode");
            buildGallery();
            break;
        default:
            LOGI("SelectionMode");
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
                LOGI("new active cam: " << i);
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
