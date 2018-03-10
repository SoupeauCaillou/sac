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

#include "base/Log.h"
#include "base/PlacementHelper.h"
#include "base/TouchInputManager.h"
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
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
#endif
}

}
