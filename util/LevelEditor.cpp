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
#include "base/Game.h"
#include "util/Draw.h"
#include "api/KeyboardInputHandlerAPI.h"

#include <SDL/SDL_keysym.h>
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

static bool gridVisible = false;
static void showGrid();
static void hideGrid();

glm::vec2 LevelEditor::GameViewPosition() {
    return glm::vec2(
        DebugAreaWidth * 0.25,
        DebugAreaHeight * 0.5);
}

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
    const char* n = theEntityManager.entityName(e);
    auto* group = strchr(n, '/');
    if (group) n += group - n + 1;
    s << n << ' ' << (e & 0xf7ffffff);
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
};

std::list<Entity> selected;

static void createTweakBarForEntity(Entity e, const std::string& barName) {
    auto it = std::find(selected.begin(), selected.end(), e);
    bool s = (it != selected.end());
    ImGui::Checkbox("Select", &s);
    if (s && it == selected.end()) {
        selected.push_back(e);
    } else if (!s && it != selected.end()) {
        selected.erase(it);
    }

    std::vector<hash_t> systems = ComponentSystem::registeredSystemIds();
    for (unsigned i=0; i<systems.size(); i++) {
        ComponentSystem* system = ComponentSystem::GetById(systems[i]);
        system->addEntityPropertiesToBar(e, 0);
    }
}



namespace EntityListMode {
    enum Enum {
        All = 0,
        VisibleOnly,
        UnderMouse
    };
}
EntityListMode::Enum listMode = EntityListMode::All;

LevelEditor::LevelEditor(Game* _game) {
    datas = new LevelEditorDatas();
    datas->activeCameraIndex = 0;
    datas->mode = EditorMode::Selection;
    datas->selectionColorChangeSpeed = -0.5;
    game = _game;

}

LevelEditor::~LevelEditor() {
    delete datas;
}

static void DumpSystemEntities(void *clientData) {
    ComponentSystem* s = ComponentSystem::GetById(*((hash_t*) clientData));

    LOGW((char*) clientData << " system dump");
    LOGW("##########################################################");
    s->forEachEntityDo([] (Entity e) -> void {
        LOGW("   " << theEntityManager.entityName(e));
    });
    LOGW("##########################################################");
}

void LevelEditor::init(KeyboardInputHandlerAPI *k) {
    kb = k;

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

namespace Tool {
    enum Enum {
        None,
        Select,
        Move,
        Rotate,
        Scale
    };
}
Tool::Enum tool = Tool::None;

void LevelEditor::tick(float dt) {
    // build entity-list Window
    std::vector<Entity> entities = theEntityManager.allEntities();
    ImGui::Begin("Entity List", NULL, ImVec2(DebugAreaWidth * 0.75, ImGui::GetIO().DisplaySize.y), -1.0f,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - DebugAreaWidth * 0.75, 0));

    std::map<hash_t, char*> groupsName;
    std::map<hash_t, std::vector<Entity>> groups;

    // CollapsingHeader
    for (unsigned i=0; i<entities.size(); i++) {
        Entity e = entities[i];

        const char* n = theEntityManager.entityName(e);
        const char* group = strchr(n, '/');
        if (group) {
            hash_t h = Murmur::RuntimeHash(n, group - n);
            groups[h].push_back(e);
            if (groupsName.find(h) == groupsName.end()) {
                char* g = strdup(n);
                g[group - n] = '\0';
                groupsName.insert(std::make_pair(h, g));
            }
        } else {
            groups[0].push_back(e);
        }
    }

    for (const auto& p: groups) {
        if (p.first == 0 || ImGui::TreeNode(groupsName[p.first])) {
            const auto& v = p.second;
            for (auto e: v) {
                std::stringstream n;
                n << entityToName(e);

                bool highLight = false;
                if (ImGui::TreeNode(n.str().c_str())) {
                    highLight = ImGui::IsHovered();
                    bool keepOpen = true;
                    showEntityWindow[e] = true;
                    createTweakBarForEntity(e,"");
                    ImGui::TreePop();
                } else {
                    highLight = ImGui::IsHovered();
                }

                if (highLight) {
                    auto* rc = theRenderingSystem.Get(e, false);
                    if (rc) rc->highLight = true;
                    auto* tc = theTextSystem.Get(e, false);
                    if (tc) tc->highLight = true;
                }
            }
            if (p.first) ImGui::TreePop();
        }

        if (p.first != 0) {
            free (groupsName[p.first]);
        }

    }

/*
    for (auto& p: showEntityWindow) {
        if (p.second) {
            Entity e = p.first;
            ImGui::BeginChild(entityToName(e).c_str());//, &p.second);

            createTweakBarForEntity(e, "");

            ImGui::EndChild();
        }
    }
*/
    imguiInputFilter();
    ImGui::End();

    ImGui::Begin("Editor tools", NULL, ImVec2(DebugAreaWidth * 0.25, ImGui::GetIO().DisplaySize.y), -1.0f,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 0));

    /* Entity manipulation tools */
    if (ImGui::CollapsingHeader("Active tool", NULL, true, true)) {
        Tool::Enum newTool = Tool::None;
        if (tool == Tool::Select) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
        if (ImGui::Button("Select (B)") || kb->isKeyReleased(Key::ByName(SDLK_b))) {
            newTool = Tool::Select;
        }
        if (tool == Tool::Select) ImGui::PopStyleColor();

        if (tool == Tool::Move) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
        if (ImGui::Button("Move (G)") || kb->isKeyReleased(Key::ByName(SDLK_g))) {
            newTool = Tool::Move;
        }
        if (tool == Tool::Move) ImGui::PopStyleColor();

        if (tool == Tool::Rotate) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
        if (ImGui::Button("Rotate (R)") || kb->isKeyReleased(Key::ByName(SDLK_r))) {
            newTool = Tool::Rotate;
        }
        if (tool == Tool::Rotate) ImGui::PopStyleColor();

        if (tool == Tool::Scale) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
        if (ImGui::Button("Scale (S)") || kb->isKeyReleased(Key::ByName(SDLK_s))) {
            newTool = Tool::Scale;
        }
        if (tool == Tool::Scale) ImGui::PopStyleColor();

        if ((newTool == tool && newTool != Tool::None) || kb->isKeyReleased(Key::ByName(SDLK_ESCAPE))) {
            tool = Tool::None;
        } else if (newTool != Tool::None) {
            tool = newTool;
        }
    }

    /* Time manipulation tools */
    if (ImGui::CollapsingHeader("Time control", NULL, true, true)) {

        switch (game->gameType) {
            case GameType::LevelEditor:
                if (ImGui::Button("Play (F1)")) game->gameType = GameType::Default;
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
                ImGui::Button("Pause (F2)");
                ImGui::PopStyleColor();
                if (ImGui::Button("Single-Step (F3)")) game->gameType = GameType::SingleStep;
                break;
            case GameType::SingleStep:
                game->gameType = GameType::SingleStep;
                break;
            default:
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
                ImGui::Button("Play (F1)");
                ImGui::PopStyleColor();
                if (ImGui::Button("Pause (F2)")) game->gameType = GameType::LevelEditor;
                if (ImGui::Button("Single-Step (F3)")) game->gameType = GameType::SingleStep;
        }
    }

    /* Rendering debug tools */
    if (ImGui::CollapsingHeader("Rendering", NULL, true, true)) {
        if (gridVisible) {
            if (ImGui::Button("Hide Grid")) {
                hideGrid();
                gridVisible = false;
            }
        } else {
            if (ImGui::Button("Show Grid")) {
                showGrid();
                gridVisible = true;
            }
        }

        ImGui::Checkbox("Wireframe", &theRenderingSystem.wireframe);
        ImGui::Checkbox("Mark opaque", &theRenderingSystem.highLight.opaque);
        ImGui::Checkbox("Mark alpha-blended", &theRenderingSystem.highLight.nonOpaque);
        ImGui::Checkbox("Mark runtime opaque", &theRenderingSystem.highLight.runtimeOpaque);
        ImGui::Checkbox("Mark z prepass", &theRenderingSystem.highLight.zPrePass);

        //TwAddVarCB(bar, "Show grid", TW_TYPE_BOOLCPP, setShowGridCallback, getShowGridCallback, 0, 0 );
    }

    imguiInputFilter();
    ImGui::End();
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

static void showGrid() {
    glm::vec2 topLeft = glm::vec2(-3 * theRenderingSystem.screenW, 3 * theRenderingSystem.screenH);
    topLeft.x = floor(topLeft.x); topLeft.y = ceil(topLeft.y);
    for (int i=0; i<=ceil(theRenderingSystem.screenW * 6); i++) {
        Draw::Vec2(HASH("__grid", 0xc37ceaec), topLeft + glm::vec2(i, 0), glm::vec2(0, -theRenderingSystem.screenH * 6), Color(0.2, 0.2, 0.2, 0.2));

        /*for (int j=1; j<=4; j++) {
            Draw::Vec2(HASH("__grid", 0xc37ceaec), topLeft + glm::vec2(i + j * 0.2, 0), glm::vec2(0, -theRenderingSystem.screenH * 6), Color(0.2, 0.2, 0.2, 0.08));
        }*/
    }

    for (int i=0; i<=ceil(theRenderingSystem.screenH * 6); i++) {
        Draw::Vec2(HASH("__grid", 0xc37ceaec), topLeft + glm::vec2(0, -i), glm::vec2(theRenderingSystem.screenW * 6, 0), Color(0.2, 0.2, 0.2, 0.2));

        /*for (int j=1; j<=4; j++) {
            Draw::Vec2(HASH("__grid", 0xc37ceaec), topLeft + glm::vec2(0, -i - j * 0.2), glm::vec2(theRenderingSystem.screenW * 6, 0), Color(0.2, 0.2, 0.2, 0.08));
        }*/
    }
}

static void hideGrid() {
    Draw::Clear(HASH("__grid", 0xc37ceaec));
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
