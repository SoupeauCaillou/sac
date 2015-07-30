#include "base/PlacementHelper.h"
#include "base/TouchInputManager.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"
#if SAC_INGAME_EDITORS
#include "util/LevelEditor.h"
#include "imgui.h"
#endif



namespace sac
{

void setResolution(int width, int height) {
    if (width < height) {
        PlacementHelper::ScreenSize = glm::vec2(10.f * width / (float)height, 10.f);
        PlacementHelper::GimpSize = glm::vec2(800.0f, 1280.0f);
    } else {
        PlacementHelper::ScreenSize = glm::vec2(/* 12.5f * width / (float)height, 12.5f); */ 20, 20.f * height / (float)width);
        PlacementHelper::GimpSize = glm::vec2(1280, 800.0f);
    }

    PlacementHelper::WindowSize = glm::vec2(width, height);

    theRenderingSystem.setWindowSize(PlacementHelper::WindowSize, PlacementHelper::ScreenSize);
    theTouchInputManager.init(PlacementHelper::ScreenSize, PlacementHelper::WindowSize);


    LOGI("Resolution set to: " << __(width) << ',' << __(height));
#if SAC_INGAME_EDITORS
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
#endif
}

}
