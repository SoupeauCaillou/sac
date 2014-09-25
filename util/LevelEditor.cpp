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

#define LEFT_PROPORTION  0.35
#define RIGHT_PROPORTION 0.65

glm::vec2 LevelEditor::GameViewPosition() {
    return glm::vec2(
        DebugAreaWidth * LEFT_PROPORTION,
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

    void updateModeSelection(float dt, const glm::vec2& mouseWorldPos, int wheelDiff);
};

std::vector<Entity> selected;
std::vector<TransformationComponent> selectedInitialTransformation;
static void markEntities(Entity* begin, int count, Color color);

static void rememberInitialTransformation() {
    selectedInitialTransformation.clear();
    for (unsigned i=0; i<selected.size(); i++) {
        selectedInitialTransformation.push_back(*TRANSFORM(selected[i]));
    }
}

static void resetTransformations() {
    for (unsigned i=0; i<selectedInitialTransformation.size(); i++) {
        *TRANSFORM(selected[i]) = selectedInitialTransformation[i];
    }
}

static void createTweakBarForEntity(Entity e) {
    if (theTransformationSystem.Get(e, false)) {
        auto it = std::find(selected.begin(), selected.end(), e);
        bool s = (it != selected.end());
        ImGui::Checkbox("Select", &s);
        if (s && it == selected.end()) {
            selected.push_back(e);
        } else if (!s && it != selected.end()) {
            selected.erase(it);
        }
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

static glm::vec2 initialCursorPosition;


void LevelEditor::tick(float dt) {
    Draw::Clear(HASH("__/mark", 0x683fdb7d));

    // build entity-list Window
    std::vector<Entity> entities = theEntityManager.allEntities();
    ImGui::Begin("Entity List", NULL, ImVec2(DebugAreaWidth * RIGHT_PROPORTION, ImGui::GetIO().DisplaySize.y), -1.0f,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - DebugAreaWidth * RIGHT_PROPORTION, 0));

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
            bool highLightAllGroup = (p.first && ImGui::IsHovered() && strcmp(groupsName[p.first], "__") != 0);

            const auto& v = p.second;
            for (auto e: v) {
                std::stringstream n;
                n << entityToName(e);

                bool highLight = highLightAllGroup;
                if (ImGui::TreeNode(n.str().c_str())) {
                    highLight |= ImGui::IsHovered();
                    createTweakBarForEntity(e);
                    ImGui::TreePop();
                } else {
                    highLight |= ImGui::IsHovered();
                }

                if (highLight && theTransformationSystem.Get(e, false)) {
                    markEntities(&e, 1, Color(1, 0, 0));
                }
            }
            if (p.first) ImGui::TreePop();
        } else {
            if (ImGui::IsHovered() && strcmp(groupsName[p.first], "__") != 0) {
                // mark all group
                const auto& v = p.second;
                for (auto e: v) {
                    markEntities(&e, 1, Color(1, 0, 0));
                }
            }
        }

        if (p.first != 0) {
            free (groupsName[p.first]);
        }

    }

    imguiInputFilter();
    ImGui::End();

    ImGui::Begin("Editor tools", NULL, ImVec2(DebugAreaWidth * LEFT_PROPORTION, ImGui::GetIO().DisplaySize.y), -1.0f,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 0));

    if (game->gameType == GameType::LevelEditor) {

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
                if (!selected.empty())
                    newTool = Tool::Move;
            }
            if (tool == Tool::Move) ImGui::PopStyleColor();

            if (tool == Tool::Rotate) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
            if (ImGui::Button("Rotate (R)") || kb->isKeyReleased(Key::ByName(SDLK_r))) {
                if (!selected.empty())
                    newTool = Tool::Rotate;
            }
            if (tool == Tool::Rotate) ImGui::PopStyleColor();

            if (tool == Tool::Scale) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 1, 0, 0.5));
            if (ImGui::Button("Scale (S)") || kb->isKeyReleased(Key::ByName(SDLK_s))) {
                if (!selected.empty())
                    newTool = Tool::Scale;
            }
            if (tool == Tool::Scale) ImGui::PopStyleColor();

            if ((newTool == tool && newTool != Tool::None)
                || kb->isKeyReleased(Key::ByName(SDLK_ESCAPE))) {
                // cancel action
                tool = Tool::None;
                resetTransformations();
            } else if (tool != Tool::None && theTouchInputManager.hasClicked(0)) {
                // confirm action
                tool = Tool::None;
                selectedInitialTransformation.clear();
                theTouchInputManager.resetState();

            } else if (newTool != Tool::None) {
                tool = newTool;
                initialCursorPosition = theTouchInputManager.getOverLastPosition();
                resetTransformations();
                rememberInitialTransformation();
            }
        }

        const glm::vec2& mouseWorldPos = theTouchInputManager.getOverLastPosition();
        switch (tool) {
            case Tool::Move : {
                for (unsigned i=0; i<selected.size(); i++) {
                    TRANSFORM(selected[i])->position =
                        selectedInitialTransformation[i].position + mouseWorldPos - initialCursorPosition;
                }
                break;
            }
            case Tool::Rotate : {
                for (unsigned i=0; i<selected.size(); i++) {
                    glm::vec2 diff[2] = {
                        initialCursorPosition - TRANSFORM(selected[i])->position,
                        mouseWorldPos - TRANSFORM(selected[i])->position
                    };
                    TRANSFORM(selected[i])->rotation =
                        selectedInitialTransformation[i].rotation + glm::atan(diff[1].y, diff[1].x) - glm::atan(diff[0].y, diff[0].x);
                }
                break;
            }
            case Tool::Scale : {
                for (unsigned i=0; i<selected.size(); i++) {
                    float scale =
                        glm::distance(mouseWorldPos, TRANSFORM(selected[i])->position) /
                        glm::max(0.01f, glm::distance(initialCursorPosition, TRANSFORM(selected[i])->position));

                    TRANSFORM(selected[i])->size =
                        selectedInitialTransformation[i].size * scale;
                }
            }
        }

    }

    {
        float c = ((int)(2 * TimeUtil::GetTime()) % 2) ? 1.0f : 0.0f;

        markEntities(&selected[0], selected.size(), Color(c,c,c,1.0f));
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
    }

    /* Camera control */
    if (ImGui::CollapsingHeader("Cameras", NULL, true, true)) {
        static int currentCamera = 0;

        const auto& cameras = theCameraSystem.RetrieveAllEntityWithComponent();
        /* assume 1 camera. TODO: add Combo to select active camera */
        static float zoom = -1.0f;
        static glm::vec2 originalSize = glm::vec2(0.0f);
        if (zoom < 0) {
            zoom = 1.0f;
            originalSize = TRANSFORM(cameras[0])->size;
        }
        if (ImGui::SliderFloat("Zoom", &zoom, 0.1, 10, "%.1f")) {
            TRANSFORM(cameras[0])->size = originalSize / zoom;
        }
        ImGui::SliderFloat2("Position", &TRANSFORM(cameras[0])->position.x, -30, 30, "%.1f");
    }

    if (ImGui::CollapsingHeader("Input Position", NULL, true, true)) {

        const glm::vec2& worldPosition = theTouchInputManager.getOverLastPosition();
        ImGui::Text("Wor");

        ImGui::SameLine();
        ImGui::Columns(2, NULL, false);
        ImGui::Value("x", worldPosition.x, "%.2f");
        ImGui::NextColumn();
        ImGui::Value("y", worldPosition.y, "%.2f");
        ImGui::Columns(1);

        const glm::vec2& screenPosition = theTouchInputManager.getOverLastPositionScreen();
        ImGui::Text("Scr");

        ImGui::SameLine();
        ImGui::Columns(2, NULL, false);
        ImGui::Value("x", screenPosition.x, "%.2f");
        ImGui::NextColumn();
        ImGui::Value("y", screenPosition.y, "%.2f");
        ImGui::Columns(1);

        const ImVec2& windowPosition = ImGui::GetIO().MousePos;
        ImGui::Text("Win");
        ImGui::SameLine();
        ImGui::Columns(2, NULL, false);
        ImGui::Value("x", (int)windowPosition.x);
        ImGui::NextColumn();
        ImGui::Value("y", (int)windowPosition.y);

        ImGui::Columns(1);
    }

    imguiInputFilter();
    ImGui::End();
}

static void markEntities(Entity* begin, int count, Color color) {
    const float thickness = 0.05;
    const  glm::vec2 corners[] = {
        glm::vec2(-0.5,  0.5), // top-left
        glm::vec2( 0.5,  0.5), // top-right
        glm::vec2(-0.5, -0.5), // bottom-left
        glm::vec2( 0.5, -0.5)  // bottom-right
    };

    for (int i=0; i<count; i++) {
        Entity e = begin[i];
        // draw 4 corners
        const auto* tc = TRANSFORM(e);

        float length = glm::max(0.1, glm::min(tc->size.x, tc->size.y) * 0.2);

        const glm::vec2 directions[] = {
            glm::vec2(length, thickness), glm::vec2(-thickness, -length),
            glm::vec2(-length, thickness), glm::vec2(thickness, -length),
            glm::vec2(length, -thickness), glm::vec2(-thickness, length),
            glm::vec2(-length, -thickness), glm::vec2(thickness, length),
        };
        for (int i=0; i<4; i++) {
            const glm::vec2 corner(tc->position + glm::rotate(tc->size * corners[i], tc->rotation));
            for (int j=0; j<2; j++) {
                Draw::Vec2(HASH("__/mark", 0x683fdb7d), corner, glm::rotate(directions[2 * i + j], tc->rotation), color);
            }
        }
    }
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
        Color c(0.2, 0.2, 0.2, 0.2);
        if (i == ceil(theRenderingSystem.screenW * 6) / 2) {
            c.g = c.b = 0.1; c.a = 0.5;
        }
        Draw::Vec2(HASH("__/grid", 0xa467b7c), topLeft + glm::vec2(i, 0), glm::vec2(0, -theRenderingSystem.screenH * 6), c);

        /*for (int j=1; j<=4; j++) {
            Draw::Vec2(HASH("__grid", 0xc37ceaec), topLeft + glm::vec2(i + j * 0.2, 0), glm::vec2(0, -theRenderingSystem.screenH * 6), Color(0.2, 0.2, 0.2, 0.08));
        }*/
    }

    for (int i=0; i<=ceil(theRenderingSystem.screenH * 6); i++) {
        Color c(0.2, 0.2, 0.2, 0.2);
        if (i == ceil(theRenderingSystem.screenH * 6) / 2) {
            c.g = c.b = 0.1; c.a = 0.5;
        }
        Draw::Vec2(HASH("__/grid", 0xa467b7c), topLeft + glm::vec2(0, -i), glm::vec2(theRenderingSystem.screenW * 6, 0), c);

        /*for (int j=1; j<=4; j++) {
            Draw::Vec2(HASH("__grid", 0xc37ceaec), topLeft + glm::vec2(0, -i - j * 0.2), glm::vec2(theRenderingSystem.screenW * 6, 0), Color(0.2, 0.2, 0.2, 0.08));
        }*/
    }
}

static void hideGrid() {
    Draw::Clear(HASH("__/grid", 0xa467b7c));
}

#endif
