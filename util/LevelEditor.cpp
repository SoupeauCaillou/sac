#ifndef EMSCRIPTEN

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

namespace EditorMode {
    enum Enum {
        Selection,
        Gallery
    };
}

struct LevelEditor::LevelEditorDatas {
    EditorMode::Enum mode;

    Entity over;
    Entity selected, gallerySelected;
    Entity selectionDisplay, overDisplay;
    Color originalColor;
    float selectionColorChangeSpeed;

    Vector2 lastMouseOverPosition;
    Vector2 selectedOriginalPos;

    unsigned activeCameraIndex;
    bool spaceWasPressed;

    Entity gallery;
    std::vector<Entity> galleryItems;

    void changeMode(EditorMode::Enum newMode);
    void buildGallery();
    void destroyGallery();

    void updateModeSelection(float dt, const Vector2& mouseWorldPos, int wheelDiff);
    void updateModeGallery(float dt, const Vector2& mouseWorldPos, int wheelDiff);

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
    barName << theEntityManager.entityName(e);
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

void LevelEditor::LevelEditorDatas::select(Entity e) {
    showTweakBarForEntity(e);
    TRANSFORM(selectionDisplay)->parent = e;
    TRANSFORM(selectionDisplay)->size = TRANSFORM(e)->size + Vector2(0.1);
    RENDERING(selectionDisplay)->hide = false;
    originalColor = RENDERING(e)->color;
    // RENDERING(selectionDisplay)->color = Color(1, 0, 0, 0.7);
}
void LevelEditor::LevelEditorDatas::deselect(Entity e) {
    RENDERING(selectionDisplay)->hide = true;
}

LevelEditor::LevelEditor() {
    datas = new LevelEditorDatas();
    datas->activeCameraIndex = 0;
    datas->mode = EditorMode::Selection;
    datas->selectionColorChangeSpeed = -0.5;
    datas->selectionDisplay = theEntityManager.CreateEntity();
    ADD_COMPONENT(datas->selectionDisplay, Rendering);
    ADD_COMPONENT(datas->selectionDisplay, Transformation);
    TRANSFORM(datas->selectionDisplay)->z = -0.01;
    RENDERING(datas->selectionDisplay)->color = Color(1, 0, 0, 0.7);

    datas->overDisplay = theEntityManager.CreateEntity();
    ADD_COMPONENT(datas->overDisplay, Rendering);
    ADD_COMPONENT(datas->overDisplay, Transformation);
    TRANSFORM(datas->overDisplay)->z = -0.01;
    RENDERING(datas->overDisplay)->color = Color(0, 0, 1, 0.7);

    TwInit(TW_OPENGL, NULL);

    glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW);
    glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
    glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
    glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
    glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);
}

LevelEditor::~LevelEditor() {
    delete datas;
}

void LevelEditor::tick(float dt) {
    std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    Entity camera = 0;
    for (unsigned i=0; i<cameras.size(); i++) {
        if (CAMERA(cameras[i])->fb == DefaultFrameBufferRef) {
            camera = cameras[i];
            break;
        }
    }
    if (!camera) {
        LOG(ERROR) << "No active camera";
        return;
    }

    int x, y;
    glfwGetMousePos(&x, &y);
    Vector2 windowPos(x / (float)PlacementHelper::WindowWidth - 0.5, 0.5 - y / (float)PlacementHelper::WindowHeight);

    const Vector2 position = TRANSFORM(camera)->worldPosition + Vector2::Rotate(windowPos * TRANSFORM(camera)->size, TRANSFORM(camera)->worldRotation);
    LOG_EVERY_N(INFO, 100) << "Mouse position: " << position;

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
    Vector2 gallerySize = Vector2(width, (textureCount / elementPerRow) * (1 * texSize));;

    gallery = theEntityManager.CreateEntity();
    ADD_COMPONENT(gallery, Transformation);
    TRANSFORM(gallery)->position = theRenderingSystem.cameras[activeCameraIndex].worldPosition;
    TRANSFORM(gallery)->z = 0.99;
    TRANSFORM(gallery)->size = gallerySize;
    ADD_COMPONENT(gallery, Rendering);
    RENDERING(gallery)->hide = false;
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
        const Vector2& te = theRenderingSystem.getTextureSize(it->first);
        if (te.X >= te.Y)
            TRANSFORM(e)->size = Vector2(texSize, texSize * te.Y / te.X);
        else
            TRANSFORM(e)->size = Vector2(texSize * te.X / te.Y, texSize);
        TRANSFORM(e)->position = Vector2(column * texSize * 1, row * texSize * 1) - gallerySize * 0.5 + Vector2(texSize * 0.5);
        ADD_COMPONENT(e, Rendering);
        RENDERING(e)->hide = false;
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

void LevelEditor::LevelEditorDatas::updateModeSelection(float dt, const Vector2& mouseWorldPos, int wheelDiff) {
#if 1
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
                if (RENDERING(entities[i])->hide)
                    continue;
                if (IntersectionUtil::pointRectangle(mouseWorldPos, TRANSFORM(entities[i])->worldPosition, TRANSFORM(entities[i])->size)) {
                    float d = Vector2::DistanceSquared(mouseWorldPos, TRANSFORM(entities[i])->worldPosition);
                    if (d < nearest) {
                        over = entities[i];
                        nearest = d;
                    }
                }
            }

            if (over) {
                TRANSFORM(overDisplay)->parent = over;
                TRANSFORM(overDisplay)->size = TRANSFORM(over)->size + Vector2(0.1);
                RENDERING(overDisplay)->hide = false;
            } else {
                RENDERING(overDisplay)->hide = true;
            }
        } else {
            if (over) {
                if (selected)
                    deselect(selected);
                selected = over;
                LOG(INFO) << "Selected entity: '" << theEntityManager.entityName(selected) << "'";
                select(selected);
                over = 0;
                RENDERING(overDisplay)->hide = true;
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
            RENDERING(e)->hide = false;
            selected = e;
        } else
            changeMode(EditorMode::Gallery);
    }
#endif
#endif
}

void LevelEditor::LevelEditorDatas::updateModeGallery(float dt, const Vector2& mouseWorldPos, int wheelDiff) {
#if 0
    if (glfwGetKey(GLFW_KEY_SPACE)) {
        spaceWasPressed = true;
    } else if (spaceWasPressed) {
        spaceWasPressed = false;
        changeMode(EditorMode::Selection);
    }

    if (wheelDiff) {
        float speed = wheelDiff * theRenderingSystem.cameras[activeCameraIndex].worldSize.Y * ( glfwGetKey( GLFW_KEY_LSHIFT ) ? 2 : 0.8);
        TRANSFORM(gallery)->position.Y -= speed * dt;
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
