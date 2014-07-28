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

GameCenterAPIHelper::GameCenterAPIHelper() :
    signButton(0), achievementsButton(0), leaderboardsButton(0), gameCenterAPI(0), bUIdisplayed(false), displayIfNotConnected(false) {}

void GameCenterAPIHelper::init(GameCenterAPI * g, bool useAchievements, bool displayIfNotConnected,
 bool useLeaderboards, std::function<void()> f) {
    gameCenterAPI = g;

    bUIdisplayed = false;

    //create entities
    leaderboardsButton = !useLeaderboards ? 0 : theEntityManager.CreateEntityFromTemplate("googleplay/leaderboards_button");
    achievementsButton = !useAchievements ? 0 : theEntityManager.CreateEntityFromTemplate("googleplay/achievements_button");
    signButton = theEntityManager.CreateEntityFromTemplate("googleplay/signinout_button");

    onLeaderboardClick = f;

    this->displayIfNotConnected = displayIfNotConnected;
}


void GameCenterAPIHelper::displayFeatures(bool display) {
    if (achievementsButton) {
        RENDERING(achievementsButton)->show =
            BUTTON(achievementsButton)->enabled = display;
        RENDERING(achievementsButton)->color.a = RENDERING(signButton)->color.a;
    }
    if (leaderboardsButton) {
        RENDERING(leaderboardsButton)->show =
            BUTTON(leaderboardsButton)->enabled = display;
        RENDERING(leaderboardsButton)->color.a = RENDERING(signButton)->color.a;
    }
}

void GameCenterAPIHelper::displayUI(float alpha) {
    LOGF_IF (! gameCenterAPI, "Asked to display GameCenter UI but it was not correctly initialized" );

    bUIdisplayed = true;
    RENDERING(signButton)->show = BUTTON(signButton)->enabled = true;
    RENDERING(signButton)->color.a = alpha;

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

void GameCenterAPIHelper::hideUI(float alpha) {
    LOGF_IF (! gameCenterAPI, "Asked to display GameCenter UI but it was not correctly initialized" );

    bUIdisplayed = false;
    BUTTON(signButton)->enabled = false;
    RENDERING(signButton)->color.a = alpha;
    RENDERING(signButton)->show = RENDERING(signButton)->color.a != 0;

    displayFeatures(RENDERING(signButton)->show);
}

bool GameCenterAPIHelper::updateUI() {
    LOGF_IF (! gameCenterAPI, "Asked to display GameCenter UI but it was not correctly initialized" );

    bool isConnected = gameCenterAPI->isConnected();

    displayFeatures((bUIdisplayed & isConnected)||displayIfNotConnected);

    if (isConnected) {
        BUTTON(signButton)->textureInactive = HASH("gg_signout", 0x35df977f);
        BUTTON(signButton)->textureActive = HASH("gg_signout_active", 0x75370bbb);
    } else {
        BUTTON(signButton)->textureInactive = HASH("gg_signin", 0x10d1e430);
        BUTTON(signButton)->textureActive = HASH("gg_signin_active", 0xcf611730);
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
