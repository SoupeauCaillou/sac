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

#include "GameCenterAPIDebugImpl.h"

#include "base/PlacementHelper.h"
#include "base/Frequency.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/AutoDestroySystem.h"

Entity GameCenterAPIDebugImpl::createAutodestroySuccess(float duration) {
    static float lastCreatedDestroySchedule = 0;

    Entity e = theEntityManager.CreateEntity("Success");
    
    ADD_COMPONENT(e, Transformation);
    ADD_COMPONENT(e, Rendering);
    ADD_COMPONENT(e, Text);
    ADD_COMPONENT(e, AutoDestroy);

    TRANSFORM(e)->size = glm::vec2(4,1);    
    TRANSFORM(e)->position = glm::vec2(0, PlacementHelper::ScreenSize.y / 2. - .5);
    TRANSFORM(e)->z = .95;

    RENDERING(e)->color = Color::random();
    RENDERING(e)->show = true;

    TEXT(e)->show = true;
    TEXT(e)->flags = TextComponent::AdjustHeightToFillWidthBit;

    AUTO_DESTROY(e)->type = AutoDestroyComponent::LIFETIME;


    // if there is already an achievement scheduled, delay this one
    float dieTime = duration + 
        std::max(0.f, lastCreatedDestroySchedule - TimeUtil::GetTime());

    AUTO_DESTROY(e)->params.lifetime.freq = Frequency<float> (dieTime);

    lastCreatedDestroySchedule = TimeUtil::GetTime() + dieTime;

    return e;
}

void GameCenterAPIDebugImpl::displayAction(float duration) {
    LOGI(message.str());
    TEXT(createAutodestroySuccess(duration))->text = message.str();

    message.clear();
    message.str("");
}

void GameCenterAPIDebugImpl::connectOrRegister() {
    _isConnected = true; 
    message << "Connected";
    displayAction();
}
void GameCenterAPIDebugImpl::disconnect() {
    _isConnected = false; 
    message << "Disconnected";
    displayAction();
}

bool GameCenterAPIDebugImpl::isConnected() {
    return _isConnected; 
}
bool GameCenterAPIDebugImpl::isRegistered() {
    return true; 
}

void GameCenterAPIDebugImpl::unlockAchievement(int id) {
    message << "Unlocked success " << id << "!";
    displayAction(8.f);
}
void GameCenterAPIDebugImpl::updateAchievementProgression(int id, int stepReached) {
    message << "Success " << id << " progression reached step no" << stepReached;
    displayAction();
}

void GameCenterAPIDebugImpl::submitScore(int leaderboardID, const std::string & score) {
    message << "Submit score " << score << " to leaderboard " << leaderboardID;
    displayAction();
}

void GameCenterAPIDebugImpl::openAchievement() {
    message << "openAchievement."; 
    displayAction();
}
void GameCenterAPIDebugImpl::openLeaderboards() {
    message << "openLeaderboards."; 
    displayAction();
}

void GameCenterAPIDebugImpl::openSpecificLeaderboard(int id) {
    message << "openSpecificLeaderboard." << id; 
    displayAction();
}
void GameCenterAPIDebugImpl::openDashboard() {
    message << "openDashboard."; 
    displayAction();
}
