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
#include <systems/AnchorSystem.h>
#include <systems/TransformationSystem.h>
#include <systems/RenderingSystem.h>
#include <systems/CameraSystem.h>
#include <systems/CollisionSystem.h>
#include <systems/TextSystem.h>
#include <systems/ZSQDSystem.h>
#include "base/PlacementHelper.h"
#include "base/Game.h"
#include "util/Draw.h"
#include "util/Random.h"
#include "util/Recorder.h"
#include "api/KeyboardInputHandlerAPI.h"
#include "imgui.h"
#include "base/TimeUtil.h"

#include <mutex>
#include <set>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <sstream>

#include "DebugConsole.h"
#if SAC_NETWORK
#include "../systems/NetworkSystem.h"
#include "../api/NetworkAPI.h"
#include "../api/linux/NetworkAPILinuxImpl.h"
#endif

#include <sys/stat.h>
#include <sys/types.h>

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
static glm::vec2 previousCameraPosition;

#define LEFT_PROPORTION 0.35
#define RIGHT_PROPORTION 0.65

glm::vec2 LevelEditor::GameViewPosition() {
    return glm::vec2(0, // DebugAreaWidth * LEFT_PROPORTION,
                     0);
}

namespace EditorMode {
    enum Enum { Selection, Gallery };
}

std::mutex twMutex;
static void _lock() { twMutex.lock(); }

static void _unlock() { twMutex.unlock(); }

void LevelEditor::addSDLEvent(const SDL_Event& evt) { events.push(evt); }

static std::string entityToName(Entity e) {
    std::stringstream s;
    const char* n = theEntityManager.entityName(e);
    auto* group = strchr(n, '/');
    if (group)
        n += group - n + 1;
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

void LevelEditor::lock() { _lock(); }

void LevelEditor::unlock() { _unlock(); }

static const ImVec4 activeColor(0, 1, 0, 0.5);

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

    std::vector<RenderingSystem::RenderCommand> backInTime;
    std::vector<std::pair<int, int>> backInTimeFrameOffsetSize;
    int currentBackFrame;
    int backInTimeCountOverride;
};

std::vector<Entity> selected;
std::vector<TransformationComponent> selectedInitialTransformation;
static void markEntities(Entity* begin, int count, Color color);

static void rememberInitialTransformation() {
    selectedInitialTransformation.clear();
    for (unsigned i = 0; i < selected.size(); i++) {
        selectedInitialTransformation.push_back(*TRANSFORM(selected[i]));

        auto* anchor = theAnchorSystem.Get(selected[i], false);
        if (anchor) {
            selectedInitialTransformation.back().position = anchor->position;
            selectedInitialTransformation.back().rotation = anchor->rotation;
            selectedInitialTransformation.back().z = anchor->z;
        }
    }
}

static void resetTransformations() {
    for (unsigned i = 0; i < selectedInitialTransformation.size(); i++) {
        auto* anchor = theAnchorSystem.Get(selected[i], false);
        if (!anchor) {
            *TRANSFORM(selected[i]) = selectedInitialTransformation[i];
        } else {
            anchor->position = selectedInitialTransformation[i].position;
            anchor->rotation = selectedInitialTransformation[i].rotation;
            anchor->z = selectedInitialTransformation[i].z;
        }
    }
}

static void clearSelection() {
    selected.clear();
    selectedInitialTransformation.clear();
}

static void createTweakBarForEntity(Entity e) {
    char tmp[100];
    strncpy(tmp, theEntityManager.entityName(e), 100);
    if (ImGui::InputText("name", tmp, 100)) {
        theEntityManager.renameEntity(e, Murmur::RuntimeHash(tmp));
    }

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
    for (unsigned i = 0; i < systems.size(); i++) {
        ComponentSystem* system = ComponentSystem::GetById(systems[i]);
        system->addEntityPropertiesToBar(e, 0);
    }
}

static void saveEntities(const Entity* entities, int count) {
    const char* outfolder = "/tmp/sac";
    char fullpath[1024];

    const auto& systems = ComponentSystem::registeredSystems();

    for (int i = 0; i < count; i++) {
        Entity e = entities[i];

        const char* name = theEntityManager.entityName(e);
        snprintf(fullpath, 1024, "%s/%s.entity", outfolder, name);
        // sigh
        char* slash = fullpath + 4;
        while ((slash = strchr(slash, '/'))) {
            *slash = '\0';
            int result = mkdir(fullpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            LOGV(1, "mkdir: " << fullpath << "->" << result);
            *slash = '/';
            slash++;
        }

        FILE* file = fopen(fullpath, "w");
        if (!file) {
            LOGE("Cannot create '" << fullpath << "' file for saving entity");
            continue;
        }

        for (auto& s : systems) {
            s.second->saveEntityToFile(e, file);
        }
        fflush(file);
        fclose(file);
    }
}

namespace EntityListMode {
    enum Enum { All = 0, VisibleOnly, UnderMouse };
}
EntityListMode::Enum listMode = EntityListMode::All;

LevelEditor::LevelEditor(Game* _game) {
    datas = new LevelEditorDatas();
    datas->activeCameraIndex = 0;
    datas->mode = EditorMode::Selection;
    datas->selectionColorChangeSpeed = -0.5;
    datas->currentBackFrame = -1;
    datas->backInTimeCountOverride = -1;
    game = _game;
}

LevelEditor::~LevelEditor() { delete datas; }

void LevelEditor::init(KeyboardInputHandlerAPI* k) {
    kb = k;
    ImGui::GetStyle().WindowRounding = 0;
}

#if 0
static void DumpSystemEntities(void *clientData) {
    ComponentSystem* s = ComponentSystem::GetById(*((hash_t*) clientData));

    LOGW((char*) clientData << " system dump");
    LOGW("##########################################################");
    s->forEachEntityDo([] (Entity e) -> void {
        LOGW("   " << theEntityManager.entityName(e));
    });
    LOGW("##########################################################");
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
#endif

namespace Tool {
    enum Enum { None, Select, Move, Rotate, Scale };
}
Tool::Enum tool = Tool::None;
namespace Modifier {
    enum Enum { None, Global_X, Local_X, Global_Y, Local_Y };
}
Modifier::Enum modifier = Modifier::None;

static glm::vec2 initialCursorPosition;
static std::vector<std::pair<Entity, float>> hoveredEntities;
static bool kbEnabled = false;

void LevelEditor::tick(float dt) {
    /* Process SDL keyboard events */
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureKeyboard) {
        if (!kbEnabled) {
            LOGV(1, "enable kb");
            SDL_StartTextInput();
            kbEnabled = true;
        }
    } else {
        if (kbEnabled) {
            LOGV(1, "disable kb");
            SDL_StopTextInput();
            kbEnabled = false;
        }
    }

    while (!events.empty()) {
        const SDL_Event& event = events.front();
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool down = (event.type == SDL_KEYDOWN);
            switch (event.key.keysym.sym) {
            case SDLK_TAB:
                io.KeysDown[ImGuiKey_Tab] = down;
                break;
            case SDLK_LEFT:
                io.KeysDown[ImGuiKey_LeftArrow] = down;
                break;
            case SDLK_RIGHT:
                io.KeysDown[ImGuiKey_RightArrow] = down;
                break;
            case SDLK_UP:
                io.KeysDown[ImGuiKey_UpArrow] = down;
                break;
            case SDLK_DOWN:
                io.KeysDown[ImGuiKey_DownArrow] = down;
                break;
            case SDLK_END:
                io.KeysDown[ImGuiKey_End] = down;
                break;
            case SDLK_DELETE:
                io.KeysDown[ImGuiKey_Delete] = down;
                break;
            case SDLK_BACKSPACE:
                io.KeysDown[ImGuiKey_Backspace] = down;
                break;
            case SDLK_RETURN:
                io.KeysDown[ImGuiKey_Enter] = down;
                break;
            case SDLK_ESCAPE:
                io.KeysDown[ImGuiKey_Escape] = down;
                break;
            default:
                break;
                if (event.key.keysym.sym < 255) {
                    if (event.type == SDL_KEYDOWN &&
                        isalnum(event.key.keysym.sym)) {
                        io.AddInputCharacter(event.key.keysym.sym);
                    }
                }
                break;
            }
        } else if (event.type == SDL_TEXTINPUT) {
            strncpy(io.InputCharacters, event.text.text, 16);
        }
        events.pop();
    }

    Draw::Clear(HASH("__/mark", 0x683fdb7d));

    // build entity-list Window
    const std::vector<Entity> entities = theEntityManager.allEntities();

    if (ImGui::Begin(
            "Entity List",
            NULL,
            ImVec2(DebugAreaWidth * RIGHT_PROPORTION, io.DisplaySize.y),
            -1.0f,
            ImGuiWindowFlags_ShowBorders)) {
        ImGui::PushStyleColor(ImGuiCol_CheckActive, activeColor);

        std::map<hash_t, char*> groupsName;
        std::map<hash_t, std::vector<Entity>> groups;

        // CollapsingHeader
        for (unsigned i = 0; i < entities.size(); i++) {
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

        for (const auto& p : groups) {
            if (p.first == 0 || ImGui::TreeNode(groupsName[p.first])) {
                bool highLightAllGroup =
                    (p.first && ImGui::IsHovered() &&
                     strcmp(groupsName[p.first], "__") != 0);

                const auto& v = p.second;
                for (auto e : v) {
                    std::stringstream n;

                    bool highLight = highLightAllGroup;
                    bool hovered = std::any_of(
                        hoveredEntities.begin(),
                        hoveredEntities.end(),
                        [e](const std::pair<Entity, float>& p) -> bool {
                            return p.first == e;
                        });
                    bool select =
                        (std::find(selected.begin(), selected.end(), e) !=
                         selected.end());

                    if (select)
                        n << "# ";
                    n << entityToName(e);
                    if (hovered)
                        n << " *";

                    intptr_t id = e;
                    if (ImGui::TreeNode((void*)id, "%s", n.str().c_str())) {
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
                if (p.first)
                    ImGui::TreePop();
            } else {
                if (ImGui::IsHovered() &&
                    strcmp(groupsName[p.first], "__") != 0) {
                    // mark all group
                    const auto& v = p.second;
                    for (auto e : v) {
                        markEntities(&e, 1, Color(1, 0, 0));
                    }
                }
            }

            if (p.first != 0) {
                free(groupsName[p.first]);
            }
        }
        ImGui::PopStyleColor();
    }
    ImGui::End();

    if (ImGui::Begin("Editor tools",
                     NULL,
                     ImVec2(DebugAreaWidth * LEFT_PROPORTION,
                            ImGui::GetIO().DisplaySize.y),
                     -1.0f,
                     ImGuiWindowFlags_ShowBorders)) {

        if (game->gameType == GameType::LevelEditor) {
            if (ImGui::CollapsingHeader("Save")) {
                if (ImGui::Button("Save All")) {
                    saveEntities(&entities[0], entities.size());
                }

                char tmp[100];
                snprintf(tmp, 100, "Save %d selected", (int)selected.size());
                if (ImGui::Button(tmp)) {
                    saveEntities(&selected[0], selected.size());
                }
            }

            /* Entity manipulation tools */
            if (ImGui::CollapsingHeader("Active tool", NULL, true, true)) {
                Tool::Enum newTool = Tool::None;
                if (tool == Tool::Select)
                    ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                if (ImGui::Button("Select (B)") || kb->isKeyReleased(SDLK_b)) {
                    newTool = Tool::Select;
                }
                if (tool == Tool::Select)
                    ImGui::PopStyleColor();

                if (tool == Tool::Move)
                    ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                if (ImGui::Button("Move (G)") || kb->isKeyReleased(SDLK_g)) {
                    if (!selected.empty())
                        newTool = Tool::Move;
                }
                if (tool == Tool::Move)
                    ImGui::PopStyleColor();

                if (tool == Tool::Rotate)
                    ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                if (ImGui::Button("Rotate (R)") || kb->isKeyReleased(SDLK_r)) {
                    if (!selected.empty())
                        newTool = Tool::Rotate;
                }
                if (tool == Tool::Rotate)
                    ImGui::PopStyleColor();

                if (tool == Tool::Scale)
                    ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                if (ImGui::Button("Scale (S)") || kb->isKeyReleased(SDLK_s)) {
                    if (!selected.empty())
                        newTool = Tool::Scale;
                }
                if (tool == Tool::Scale)
                    ImGui::PopStyleColor();

                if ((newTool == tool && newTool != Tool::None) ||
                    kb->isKeyReleased(SDLK_ESCAPE)) {
                    // cancel action
                    tool = Tool::None;
                    modifier = Modifier::None;
                    resetTransformations();
                } else if (tool != Tool::None &&
                           theTouchInputManager.hasClicked(0)) {
                    // confirm action
                    tool = Tool::None;
                    modifier = Modifier::None;
                    selectedInitialTransformation.clear();
                    theTouchInputManager.resetState();

                } else if (newTool != Tool::None) {
                    tool = newTool;
                    modifier = Modifier::None;
                    initialCursorPosition =
                        theTouchInputManager.getOverLastPosition();
                    resetTransformations();
                    rememberInitialTransformation();
                }
            }

            const glm::vec2& mouseWorldPos =
                theTouchInputManager.getOverLastPosition();

            if (tool == Tool::Move || tool == Tool::Scale) {
                if (kb->isKeyReleased(SDLK_x)) {
                    initialCursorPosition =
                        mouseWorldPos; // reset mouse displacement
                    switch (modifier) {
                    case Modifier::None:
                    case Modifier::Local_Y:
                    case Modifier::Global_Y:
                        modifier = Modifier::Global_X;
                        break;
                    case Modifier::Global_X:
                        modifier = Modifier::Local_X;
                        break;
                    case Modifier::Local_X:
                        modifier = Modifier::None;
                        break;
                    }
                } else if (kb->isKeyReleased(SDLK_y)) {
                    initialCursorPosition =
                        mouseWorldPos; // reset mouse displacement
                    switch (modifier) {
                    case Modifier::None:
                    case Modifier::Local_X:
                    case Modifier::Global_X:
                        modifier = Modifier::Global_Y;
                        break;
                    case Modifier::Global_Y:
                        modifier = Modifier::Local_Y;
                        break;
                    case Modifier::Local_Y:
                        modifier = Modifier::None;
                        break;
                    }
                }
            }

            switch (tool) {
            case Tool::Move: {
                glm::vec2 diff = mouseWorldPos - initialCursorPosition;
                if (kb->isKeyPressed(SDLK_LCTRL)) {
                    diff = glm::round(diff);
                }
                for (unsigned i = 0; i < selected.size(); i++) {
                    auto* anchor = theAnchorSystem.Get(selected[i], false);
                    glm::vec2* position =
                        anchor ? &(anchor->position)
                               : &TRANSFORM(selected[i])->position;

                    switch (modifier) {
                    case Modifier::None:
                        (*position) =
                            selectedInitialTransformation[i].position + diff;
                        break;
                    case Modifier::Global_X:
                        (*position).x =
                            selectedInitialTransformation[i].position.x +
                            diff.x;
                        (*position).y =
                            selectedInitialTransformation[i].position.y;
                        break;
                    case Modifier::Global_Y:
                        (*position).x =
                            selectedInitialTransformation[i].position.x;
                        (*position).y =
                            selectedInitialTransformation[i].position.y +
                            diff.y;
                        break;
                    case Modifier::Local_X:
                        (*position) =
                            selectedInitialTransformation[i].position +
                            glm::rotate(
                                glm::vec2(glm::sign(diff.x) * glm::length(diff),
                                          0.0f),
                                selectedInitialTransformation[i].rotation);
                        break;
                    case Modifier::Local_Y:
                        (*position) =
                            selectedInitialTransformation[i].position +
                            glm::rotate(
                                glm::vec2(0.0f,
                                          glm::sign(diff.y) *
                                              glm::length(diff)),
                                selectedInitialTransformation[i].rotation);
                        break;
                    }
                }
                break;
            }
            case Tool::Rotate: {
                for (unsigned i = 0; i < selected.size(); i++) {
                    glm::vec2 diff[2] = {initialCursorPosition -
                                             TRANSFORM(selected[i])->position,
                                         mouseWorldPos -
                                             TRANSFORM(selected[i])->position};
                    auto* anchor = theAnchorSystem.Get(selected[i], false);
                    float* rotation = anchor
                                          ? &(anchor->rotation)
                                          : &TRANSFORM(selected[i])->rotation;
                    (*rotation) = selectedInitialTransformation[i].rotation +
                                  glm::atan(diff[1].y, diff[1].x) -
                                  glm::atan(diff[0].y, diff[0].x);
                }
                break;
            }
            case Tool::Scale: {
                for (unsigned i = 0; i < selected.size(); i++) {
                    glm::vec2 scale = glm::vec2(
                        glm::distance(mouseWorldPos,
                                      TRANSFORM(selected[i])->position) /
                        glm::max(
                            0.01f,
                            glm::distance(initialCursorPosition,
                                          TRANSFORM(selected[i])->position)));

                    switch (modifier) {
                    case Modifier::None:
                        break;
                    case Modifier::Global_X:
                        scale.y = 1.0f;
                        break;
                    case Modifier::Global_Y:
                        scale.x = 1.0f;
                        break;
                    case Modifier::Local_X:
                    case Modifier::Local_Y:
                        LOGT("To implement");
                        break;
                    }
                    TRANSFORM(selected[i])
                        ->size = selectedInitialTransformation[i].size * scale;
                }
                break;
            }
            default:
                // hovered entities
                hoveredEntities.clear();
                for (auto e : entities) {
                    const auto* rc = theRenderingSystem.Get(e, false);
                    if (!rc)
                        continue;
                    if (!rc->show)
                        continue;

                    const auto* tc = theTransformationSystem.Get(e, false);
                    if (tc) {
                        if (IntersectionUtil::pointRectangle(mouseWorldPos,
                                                             tc)) {
                            hoveredEntities.push_back(std::make_pair(e, tc->z));
                        }
                    }
                }

                if (!hoveredEntities.empty()) {
                    std::sort(hoveredEntities.begin(),
                              hoveredEntities.end(),
                              [](const std::pair<Entity, float>& p1,
                                 const std::pair<Entity, float>& p2) -> bool {
                                  return p1.second > p2.second;
                              });

                    float c =
                        ((int)(2 * TimeUtil::GetTime()) % 2) ? 1.0f : 0.5f;
                    markEntities(&hoveredEntities[0].first,
                                 1,
                                 Color(0, c * 0.5, c, 0.5f));

                    if (theTouchInputManager.hasClicked(1)) {
                        int index = 0;
                        /* special case: if selected is hoveredEntities[0], use
                         * 1 instead */
                        if (hoveredEntities.size() > 1 &&
                            selected.size() == 1) {
                            if (selected[0] == hoveredEntities[0].first) {
                                index = 1;
                            }
                        }

                        if (!kb->isKeyPressed(SDLK_LSHIFT)) {
                            clearSelection();
                        }

                        Entity e = hoveredEntities[index].first;
                        auto it =
                            std::find(selected.begin(), selected.end(), e);
                        if (it == selected.end()) {
                            selected.push_back(e);
                        } else {
                            selected.erase(it);
                        }
                    }
                }
            }
#endif
            theAnchorSystem.Update(dt);

            if (ImGui::CollapsingHeader("Entity Builder", NULL, true, true)) {
                if (ImGui::Button("Add Entity")) {
                    /* Default components: Transformation and Rendering */
                    Entity e = theEntityManager.CreateEntity(
                        Murmur::RuntimeHash("new_entity"));
                    ADD_COMPONENT(e, Transformation);
                    TRANSFORM(e)
                        ->position =
                        TRANSFORM(
                            theCameraSystem.RetrieveAllEntityWithComponent()[0])
                            ->position;
                    TRANSFORM(e)->z = 1.0f;
                    ADD_COMPONENT(e, Rendering);
                    RENDERING(e)->show = 1;

                    selected.push_back(e);
                }
                {
                    char tmp[100];
                    snprintf(
                        tmp, 100, "Delete %d entities", (int)selected.size());
                    if (ImGui::Button(tmp)) {
                        for (auto e : selected) {
                            theEntityManager.DeleteEntity(e);
                        }
                        clearSelection();
                    }
                }

                if (!selected.empty()) {
                    if (ImGui::CollapsingHeader("Active systems")) {
                        const auto& systems =
                            ComponentSystem::registeredSystems();
                        for (const auto& p : systems) {
                            auto* sys = p.second;
                            bool on = std::all_of(
                                selected.begin(),
                                selected.end(),
                                [sys](Entity e) -> bool {
                                    return (sys->componentAsVoidPtr(e) != NULL);
                                });

                            if (ImGui::Checkbox(INV_HASH(p.first), &on)) {
                                for (auto e : selected) {
                                    if (on) {
                                        theEntityManager.AddComponent(e, sys);
                                    } else {
                                        theEntityManager.RemoveComponent(e,
                                                                         sys);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (!selected.empty()) {
            float c = ((int)(2 * TimeUtil::GetTime()) % 2) ? 1.0f : 0.0f;
            markEntities(&selected[0], selected.size(), Color(c, c, c, 1.0f));

            // draw modifier axis at entity 0, if any
            if (modifier != Modifier::None) {
                glm::vec2 axis;
                switch (modifier) {
                case Modifier::Global_X:
                    axis = glm::vec2(1.0f, 0.0f);
                    break;
                case Modifier::Global_Y:
                    axis = glm::vec2(0.0f, 1.0f);
                    break;
                case Modifier::Local_X:
                    axis = glm::rotate(glm::vec2(1.0f, 0.0f),
                                       TRANSFORM(selected[0])->rotation);
                    break;
                case Modifier::Local_Y:
                    axis = glm::rotate(glm::vec2(0.0f, 1.0f),
                                       TRANSFORM(selected[0])->rotation);
                    break;
                case Modifier::None:
                    LOGF("Should not be here");
                }

                Draw::Vec2(HASH("__/mark", 0x683fdb7d),
                           selectedInitialTransformation[0].position -
                               axis * theRenderingSystem.screenW,
                           axis * (2 * theRenderingSystem.screenW),
                           Color(1.0f, 0.0, 0.0));
            }
        }

        /* Time manipulation tools */
        {
            ImGui::CollapsingHeader("Time control", NULL, true, true);

            switch (game->gameType) {
            case GameType::LevelEditor:
                if (ImGui::Button("Play (F1)") || kb->isKeyReleased(SDLK_F1))
                    game->gameType = GameType::Default;
                ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                ImGui::Button("Editor (F2)");
                ImGui::PopStyleColor();
                if (ImGui::Button("Single-Step (F3)") ||
                    kb->isKeyReleased(SDLK_F3))
                    game->gameType = GameType::SingleStep;
                if (ImGui::Button("Replay (F4)") ||
                    kb->isKeyReleased(SDLK_F4)) {
                    datas->currentBackFrame =
                        datas->backInTimeFrameOffsetSize.size() - 1;
                    game->gameType = GameType::Replay;
                }
                break;
            case GameType::Replay:
                if (ImGui::Button("Play (F1)") || kb->isKeyReleased(SDLK_F1))
                    game->gameType = GameType::Default;
                if (ImGui::Button("Editor (F2)") ||
                    kb->isKeyReleased(SDLK_F2)) {
                    if (!gridVisible) {
                        showGrid();
                        gridVisible = true;
                    }
                    game->gameType = GameType::LevelEditor;
                }
                if (ImGui::Button("Single-Step (F3)") ||
                    kb->isKeyReleased(SDLK_F3))
                    game->gameType = GameType::SingleStep;
                ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                ImGui::Button("Replay (F4)");
                ImGui::PopStyleColor();
                break;
            case GameType::SingleStep:
                game->gameType = GameType::SingleStep;
                break;
            default:
                ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
                ImGui::Button("Play (F1)");
                ImGui::PopStyleColor();
                if (ImGui::Button("Editor (F2)") ||
                    kb->isKeyReleased(SDLK_F2)) {
                    if (!gridVisible) {
                        showGrid();
                        gridVisible = true;
                    }
                    game->gameType = GameType::LevelEditor;
                }
                if (ImGui::Button("Single-Step (F3)") ||
                    kb->isKeyReleased(SDLK_F3))
                    game->gameType = GameType::SingleStep;
                if (ImGui::Button("Replay (F4)") ||
                    kb->isKeyReleased(SDLK_F4)) {
                    datas->currentBackFrame =
                        datas->backInTimeFrameOffsetSize.size() - 1;
                    game->gameType = GameType::Replay;
                }
            }
        }

        /* Rendering debug tools */
        if (ImGui::CollapsingHeader("Rendering", NULL, true, false)) {
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
            ImGui::Checkbox("Mark opaque",
                            &theRenderingSystem.highLight.opaque);
            ImGui::Checkbox("Mark alpha-blended",
                            &theRenderingSystem.highLight.nonOpaque);
            ImGui::Checkbox("Mark runtime opaque",
                            &theRenderingSystem.highLight.runtimeOpaque);
            ImGui::Checkbox("Mark z prepass",
                            &theRenderingSystem.highLight.zPrePass);
        }

#if !DISABLE_COLLISION_SYSTEM
        /* Collision debug tools */
        if (ImGui::CollapsingHeader("Collision", NULL, true, false)) {
            ImGui::Checkbox("Debug", &theCollisionSystem.showDebug);
            ImGui::SliderInt(
                "Raycast/s", &theCollisionSystem.maximumRayCastPerSec, -1, 100);
        }
#endif

#if !DISABLE_ZSQD_SYSTEM
        /* ZSQD debug tools */
        if (ImGui::CollapsingHeader("ZSQD", NULL, true, false)) {
            ImGui::Checkbox("Debug", &theZSQDSystem.showDebug);
        }
#endif
#if SAC_NETWORK
        if (ImGui::CollapsingHeader("Network", NULL, true, true)) {
            // show stats
            ImGui::Columns(3, NULL, false);
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                               "dl %.2f",
                               theNetworkSystem.dlRate / 1000);
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                               "up %.2f",
                               theNetworkSystem.ulRate / 1000);
            ImGui::NextColumn();
            ImGui::Text(" kB/s");
            ImGui::Columns(1);
            ImGui::Separator();

            {
                /* hmmm */
                static char *nickname = NULL, *lobby = NULL;
                if (nickname == NULL && lobby == NULL) {
                    nickname = (char*)malloc(64);
                    strcpy(nickname, "anonymous");
                    lobby = (char*)malloc(64);
                    strcpy(lobby, "127.0.0.1");

                    for (int i = 0; i < game->arg.c; i++) {
                        if (!strcmp(game->arg.v[i], "--nickname"))
                            sprintf(nickname, "%.63s", game->arg.v[++i]);
                        if (!strcmp(game->arg.v[i], "--lobby"))
                            sprintf(lobby, "%.63s", game->arg.v[++i]);
                    }
                }

                NetworkAPILinuxImpl* api = static_cast<NetworkAPILinuxImpl*>(
                    theNetworkSystem.networkAPI);
                /* lobby buttons */
                switch (api->getStatus()) {
                case NetworkStatus::None:
                case NetworkStatus::ConnectionToLobbyFailed:
                case NetworkStatus::LoginFailed:
                    ImGui::InputText("nickname", nickname, 64);
                    ImGui::InputText("lobby", lobby, 64);

                    /* add button to retry connection to lobby */
                    if (ImGui::Button("Connect")) {
                        api->login(nickname, lobby);
                    }
                    break;
                case NetworkStatus::ConnectingToLobby:
                case NetworkStatus::ConnectedToLobby:
                case NetworkStatus::LoginInProgress:
                    ImGui::LabelText("nickname", nickname);
                    ImGui::LabelText("lobby", lobby);
                    ImGui::Button("Connecting...");
                    break;
                default:
                    ImGui::LabelText("nickname", nickname);
                    ImGui::LabelText("lobby", lobby);
                }
                ImGui::Separator();
                /* in-lobby buttons */
                switch (api->getStatus()) {
                case NetworkStatus::Logged:
                    /* display pending invitations if any */
                    if (api->getPendingInvitationCount()) {
                        if (ImGui::Button("Accept invit.")) {
                            api->acceptInvitation();
                        }
                    } else if (ImGui::Button("Invite")) {
                        api->createRoom();
                    }
                    break;
                case NetworkStatus::CreatingRoom:
                    ImGui::LabelText("status", "creating room");
                    break;
                case NetworkStatus::JoiningRoom:
                    ImGui::LabelText("status", "joining room");
                    break;
                case NetworkStatus::InRoomAsMaster:
                    ImGui::LabelText("status", "game master");
                    break;
                case NetworkStatus::ConnectedToServer:
                    ImGui::LabelText("status", "connected");
                    break;
                default:
                    break;
                }
                ImGui::Separator();
                switch (api->getStatus()) {
                case NetworkStatus::InRoomAsMaster:
                case NetworkStatus::ConnectedToServer: {
                    auto players = api->getPlayersInRoom();
                    for (auto it : players) {
                        switch (it.second) {
                        case NetworkStatus::JoiningRoom:
                            ImGui::LabelText(it.first.c_str(), "invited");
                            break;
                        case NetworkStatus::InRoomAsMaster:
                            ImGui::LabelText(it.first.c_str(), "master");
                            break;
                        case NetworkStatus::ConnectedToServer:
                            ImGui::LabelText(it.first.c_str(), "connected");
                            break;
                        default:
                            ImGui::LabelText(it.first.c_str(), "---");
                        }
                    }
                }
                default:
                    break;
                }
            }
        }
#endif

        if (gridVisible) {
            Entity camera = theCameraSystem.RetrieveAllEntityWithComponent()[0];
            if (glm::distance(TRANSFORM(camera)->position,
                              previousCameraPosition) > 0.01) {
                hideGrid();
                showGrid();
            }
        }

        /* Camera control */
        ImGui::CollapsingHeader("Cameras", NULL, true, true);
        {
            const auto& cameras =
                theCameraSystem.RetrieveAllEntityWithComponent();
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

            if (game->gameType == GameType::LevelEditor) {
                const glm::vec2& pos =
                    theTouchInputManager.getOverLastPosition();
                if (IntersectionUtil::pointRectangle(pos,
                                                     TRANSFORM(cameras[0])) &&
                    io.MouseWheel) {
                    zoom += 2 * io.MouseWheel * dt;
                    TRANSFORM(cameras[0])->size = originalSize / zoom;
                }
            }

            if (kb->isKeyReleased(SDLK_c)) {
                TRANSFORM(cameras[0])
                    ->position = theTouchInputManager.getOverLastPosition();
            }

            ImGui::SliderFloat2("Position",
                                &TRANSFORM(cameras[0])->position.x,
                                -30,
                                30,
                                "%.1f");
        }

        if (ImGui::CollapsingHeader("Input Position", NULL, true, true)) {

            const glm::vec2& worldPosition =
                theTouchInputManager.getOverLastPosition();
            ImGui::Text("Wor");

            ImGui::SameLine();
            ImGui::Columns(2, NULL, false);
            ImGui::Value("x", worldPosition.x, "%.2f");
            ImGui::NextColumn();
            ImGui::Value("y", worldPosition.y, "%.2f");
            ImGui::Columns(1);

            const glm::vec2& screenPosition =
                theTouchInputManager.getOverLastPositionScreen();
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

        {
            ImGui::CollapsingHeader("Record");
            static bool recordWholeWindow = true;
            if (Recorder::Instance().isRecording()) {
                if (ImGui::Button("Stop (F8)") || kb->isKeyReleased(SDLK_F8)) {
                    Recorder::Instance().stop();
                    Recorder::Instance().deinit();
                }
            } else {
                if (ImGui::Button("Start (F8)") || kb->isKeyReleased(SDLK_F8)) {
                    if (recordWholeWindow) {
                        Recorder::Instance().init();
                    } else {
                        glm::vec2 position = GameViewPosition();
                        position.y =
                            (theRenderingSystem.windowH + DebugAreaHeight) -
                            (position.y + theRenderingSystem.windowH);
                        Recorder::Instance().init(
                            position,
                            glm::vec2(theRenderingSystem.windowW,
                                      theRenderingSystem.windowH));
                    }
                    Recorder::Instance().start();
                }
            }
            ImGui::Checkbox("Include editor", &recordWholeWindow);
        }
        if (game->gameType == GameType::Replay) {
            static int playback = 0;

            ImGui::CollapsingHeader("Replay");
            ImGui::Text("Frame %d/%d",
                        datas->currentBackFrame + 1,
                        (int)datas->backInTimeFrameOffsetSize.size());
            if (playback == 0) {
                if (ImGui::Button("Backward <")) {
                    playback = -1;
                }
                if (ImGui::Button("Forward >")) {
                    playback = 1;
                }

                int inc = 1;
                if (kb->isKeyPressed(SDLK_LSHIFT)) {
                    inc = 10;
                }

                if (kb->isKeyPressed(SDLK_LCTRL)) {
                    if (kb->isKeyReleased(SDLK_LEFT))
                        datas->currentBackFrame -= inc;
                    if (kb->isKeyReleased(SDLK_RIGHT))
                        datas->currentBackFrame += inc;
                } else {
                    if (kb->isKeyPressed(SDLK_LEFT))
                        datas->currentBackFrame -= inc;
                    if (kb->isKeyPressed(SDLK_RIGHT))
                        datas->currentBackFrame += inc;
                }

            } else {
                if (ImGui::Button("Pause")) {
                    playback = 0;
                }
            }

            int old = datas->currentBackFrame;
            datas->currentBackFrame = glm::max(
                0,
                glm::min((int)datas->backInTimeFrameOffsetSize.size() - 1,
                         datas->currentBackFrame + playback));
            if (datas->currentBackFrame == old)
                playback = 0;
        }

    }
    ImGui::End();

    if (ImGui::Begin("Graphs",
                     NULL,
                     ImVec2(ImGui::GetIO().DisplaySize.x - DebugAreaWidth,
                            DebugAreaHeight),
                     -1.0f,
                     ImGuiWindowFlags_ShowBorders)) {
        if (ImGui::CollapsingHeader("Entities")) {
            static std::vector<float> count;
            count.push_back(theEntityManager.getNumberofEntity());
            if (count.size() > 300)
                count.erase(count.begin());
            ImGui::PlotLines(
                "Entities",
                &count[0],
                count.size(),
                0,
                NULL,
                0,
                FLT_MAX,
                ImVec2(DebugAreaWidth * 0.8, DebugAreaHeight * 0.7));
        }
        if (ImGui::CollapsingHeader("FPS")) {
            static std::vector<float> fps;
            fps.push_back(1.0f / dt);
            if (fps.size() > 300)
                fps.erase(fps.begin());
            ImGui::PlotLines(
                "FPS",
                &fps[0],
                fps.size(),
                0,
                NULL,
                FLT_MIN,
                FLT_MAX,
                ImVec2(DebugAreaWidth * 0.8, DebugAreaHeight * 0.7));
        }
    }
    ImGui::End();

    if (game->gameType == GameType::Replay) {
        datas->backInTimeCountOverride = -1;
        if (ImGui::Begin("Frame",
                         NULL,
                         ImVec2(DebugAreaWidth, DebugAreaHeight),
                         -1.0f,
                         ImGuiWindowFlags_ShowBorders)) {
            int previousBatch = 0;
            int batch = 0;
            ImVec4 batchColor[] = {
                ImVec4(1.0f, 0.0f, 0.0f, 1.f),
                ImVec4(0.0f, 0.0f, 1.0f, 1.f),
                ImVec4(0.0f, 1.0f, 0.0f, 1.f),
                ImVec4(0.5f, 1.0f, 0.0f, 1.f),
                ImVec4(0.0f, .5f, 0.5f, 1.f),
                ImVec4(0.5f, .5f, 0.5f, 1.f),
            };
            char id[128];

            /* display render commands */
            const auto& p = datas->backInTimeFrameOffsetSize[datas->currentBackFrame];
            for (int i=0; i<p.second; i++) {
                const RenderingSystem::RenderCommand& rc =
                    datas->backInTime[p.first + i];


                if (rc.batchIndex) {
                    if (*(rc.batchIndex) != previousBatch) {
                        previousBatch = *(rc.batchIndex);
                        batch++;
                    }
                }
                if (i >= datas->backInTimeCountOverride && datas->backInTimeCountOverride >= 0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, batchColor[5]);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, batchColor[batch % 5]);
                }

                if (rc.texture == BeginFrameMarker) {
                    sprintf(id, "%d - Begin", i);
                } else if (rc.texture == EndFrameMarker) {
                    sprintf(id, "%d - End", i);
                } else {
                    sprintf(id, "%d", i);
                }

                if (ImGui::TreeNode(id)) {
                    ImGui::PopStyleColor();
                    ImGui::Value("entity", rc.e);
                    ImGui::Value("z", rc.z);
                    ImGui::Value("effect", (unsigned int) rc.effectRef);
                    ImGui::Color("color", rc.color.asInt());
                    // state
                    ImGui::Value("z-pre-pass", (bool) (rc.flags & ZPrePassFlagSet));
                    ImGui::Value("opaque", (bool) (rc.flags & OpaqueFlagSet));
                    ImGui::Value("alpha-blended", (bool) (rc.flags & AlphaBlendedFlagSet));
                    // texture
                    if (rc.texture != InvalidTextureRef) {
                        const TextureInfo* info = theRenderingSystem.textureLibrary.get(rc.texture, false);
                        if (info) {
                            ImGui::Text("atlas: %s", theRenderingSystem.atlas[info->atlasIndex].name.c_str());
                        }
                    }
                    ImGui::TreePop();
                } else {
                    ImGui::PopStyleColor();
                }
                if (ImGui::IsHovered()) {
                    datas->backInTimeCountOverride = i + 1;
                }
            }
        }
        ImGui::End();
    }

    if (ImGui::GetIO().WantCaptureMouse) {
        // force no click state
        theTouchInputManager.resetState();
    }
}

static void markEntities(Entity* begin, int count, Color color) {
    const float thickness = 0.05;
    const glm::vec2 corners[] = {
        glm::vec2(-0.5, 0.5),  // top-left
        glm::vec2(0.5, 0.5),   // top-right
        glm::vec2(-0.5, -0.5), // bottom-left
        glm::vec2(0.5, -0.5)   // bottom-right
    };

    for (int i = 0; i < count; i++) {
        Entity e = begin[i];
        // draw 4 corners
        const auto* tc = theTransformationSystem.Get(e, false);
        if (!tc)
            continue;

        float length = glm::max(0.1, glm::min(tc->size.x, tc->size.y) * 0.2);

        const glm::vec2 directions[] = {
            glm::vec2(length, thickness),
            glm::vec2(-thickness, -length),
            glm::vec2(-length, thickness),
            glm::vec2(thickness, -length),
            glm::vec2(length, -thickness),
            glm::vec2(-thickness, length),
            glm::vec2(-length, -thickness),
            glm::vec2(thickness, length),
        };
        for (int i = 0; i < 4; i++) {
            const glm::vec2 corner(
                tc->position +
                glm::rotate(tc->size * corners[i], tc->rotation));
            for (int j = 0; j < 2; j++) {
                Draw::Vec2(HASH("__/mark", 0x683fdb7d),
                           corner,
                           glm::rotate(directions[2 * i + j], tc->rotation),
                           color);
            }
        }
    }
}

static void showGrid() {
    Entity camera = theCameraSystem.RetrieveAllEntityWithComponent()[0];
    Color line = CAMERA(camera)->clearColor;
    for (int i = 0; i < 3; i++) {
        line.rgba[i] -= 0.2;
    }

    previousCameraPosition = TRANSFORM(camera)->position;
    previousCameraPosition.x = glm::round(previousCameraPosition.x);
    previousCameraPosition.y = glm::round(previousCameraPosition.y);

    glm::vec2 bottomLeft = previousCameraPosition -
                           0.5f * glm::vec2(theRenderingSystem.screenW,
                                            theRenderingSystem.screenH);
    bottomLeft.x = floor(bottomLeft.x);
    bottomLeft.y = floor(bottomLeft.y);

    for (int i = 0; i <= ceil(theRenderingSystem.screenW + 1); i++) {
        Color c(line.r, line.g, line.b, 0.2);
        if (((int)bottomLeft.x + i) == 0) {
            c.a = 0.8;
        }
        Draw::Vec2(HASH("__/grid", 0xa467b7c),
                   bottomLeft + glm::vec2(i, 0),
                   glm::vec2(0, theRenderingSystem.screenH + 1),
                   c);
    }

    for (int i = 0; i <= theRenderingSystem.screenH + 1; i++) {
        Color c(line.r, line.g, line.b, 0.2);
        if (((int)bottomLeft.y + i) == 0) {
            c.a = 0.8;
        }
        Draw::Vec2(HASH("__/grid", 0xa467b7c),
                   bottomLeft + glm::vec2(0, i),
                   glm::vec2(theRenderingSystem.screenW + 1, 0),
                   c);
    }
}

static void hideGrid() { Draw::Clear(HASH("__/grid", 0xa467b7c)); }

void LevelEditor::newFrame(RenderingSystem::RenderCommand* commands,
                           int count) {
    if (game->gameType == GameType::Default ||
        game->gameType == GameType::SingleStep) {
        datas->backInTimeFrameOffsetSize.push_back(
            std::make_pair((int)datas->backInTime.size(), count));
        datas->backInTime.insert(
            datas->backInTime.end(), commands, commands + count);
    }
}

static int batchIndexes[1024];
RenderingSystem::RenderCommand* LevelEditor::getFrame(int* count) {
    const auto& p = datas->backInTimeFrameOffsetSize[datas->currentBackFrame];
    *count = p.second;
    if (datas->backInTimeCountOverride > 0) {
        *count = datas->backInTimeCountOverride;
    }
    auto* result = &datas->backInTime[p.first];
    for (int i=0; i<*count; i++) {
        result[i].batchIndex = &batchIndexes[i];
    }
    return result;
}
