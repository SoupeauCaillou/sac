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

#include "GameCenterAPIHelper.h"

#include "base/EntityManager.h"

#include <systems/RenderingSystem.h>
#include <systems/ButtonSystem.h>

void GameCenterAPIHelper::init(GameCenterAPI * g, bool useAchievements, bool displayIfNotConnected, 
 bool useLeaderboards, std::function<void()> f) {
    gameCenterAPI = g;

    bUIdisplayed = false;

    //create entities
    signButton = theEntityManager.CreateEntity("gg/sign_in_out_button",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("googleplay/signinout_button"));

    achievementsButton = !useAchievements ? 0 : theEntityManager.CreateEntity("gg/achievements_button",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("googleplay/achievements_button"));

    leaderboardsButton = !useLeaderboards ? 0 : theEntityManager.CreateEntity("gg/leaderboards_button",
        EntityType::Persistent, theEntityManager.entityTemplateLibrary.load("googleplay/leaderboards_button"));

    onLeaderboardClick = f;

    this->displayIfNotConnected = displayIfNotConnected;
}


void GameCenterAPIHelper::displayFeatures(bool display) {
    if (achievementsButton) {
        RENDERING(achievementsButton)->show =
            BUTTON(achievementsButton)->enabled = display;
    }
    if (leaderboardsButton) {
        RENDERING(leaderboardsButton)->show = 
            BUTTON(leaderboardsButton)->enabled = display;
    }
}

void GameCenterAPIHelper::displayUI() {
    LOGF_IF (! gameCenterAPI, "Asked to display GameCenter UI but it was not correctly initialized" );

    bUIdisplayed = true;
    RENDERING(signButton)->show = BUTTON(signButton)->enabled = true;

    //only display other buttons if we are connected
    displayFeatures(displayIfNotConnected || gameCenterAPI->isConnected());
}

void GameCenterAPIHelper::registerForFading(FaderHelper* fader, Fading::Enum type) {
    switch (type) {
        case Fading::In:
            fader->registerFadingInEntity(signButton);
            if (gameCenterAPI->isConnected()) {
                if (achievementsButton)
                    fader->registerFadingInEntity(achievementsButton);
                if (leaderboardsButton)
                    fader->registerFadingInEntity(leaderboardsButton);
            }
            break;
        case Fading::Out:
            fader->registerFadingOutEntity(signButton);
            if (achievementsButton && RENDERING(achievementsButton)->show)
                fader->registerFadingOutEntity(achievementsButton);
            if (leaderboardsButton && RENDERING(leaderboardsButton)->show)
                fader->registerFadingOutEntity(leaderboardsButton);
            break;
        default:
            LOGF("Invalid type '" << type << "' used in " << __PRETTY_FUNCTION__);
    }
}

void GameCenterAPIHelper::hideUI() {
    LOGF_IF (! gameCenterAPI, "Asked to display GameCenter UI but it was not correctly initialized" );

    bUIdisplayed = false;
    RENDERING(signButton)->show = BUTTON(signButton)->enabled = false;

    displayFeatures(false);
}  

bool GameCenterAPIHelper::updateUI() {
    LOGF_IF (! gameCenterAPI, "Asked to display GameCenter UI but it was not correctly initialized" );

    bool isConnected = gameCenterAPI->isConnected();

    displayFeatures((bUIdisplayed & isConnected)||displayIfNotConnected);
    
    if (isConnected) {
        BUTTON(signButton)->textureInactive = theRenderingSystem.loadTextureFile("gg_signout");
        BUTTON(signButton)->textureActive = theRenderingSystem.loadTextureFile("gg_signout_active");
    } else {
        BUTTON(signButton)->textureInactive = theRenderingSystem.loadTextureFile("gg_signin");
        BUTTON(signButton)->textureActive = theRenderingSystem.loadTextureFile("gg_signin_active");
    }

    if (BUTTON(signButton)->clicked) {
        isConnected ? gameCenterAPI->disconnect() : gameCenterAPI->connectOrRegister();
        return true;
    } else if (achievementsButton && BUTTON(achievementsButton)->clicked) {
        isConnected ? gameCenterAPI->openAchievement() : gameCenterAPI->connectOrRegister();
        return true;
    } else if (leaderboardsButton && BUTTON(leaderboardsButton)->clicked) {
        isConnected ? onLeaderboardClick() : gameCenterAPI->connectOrRegister();
        
        return true;
    }

    return false;
}
