#pragma once

#include "System.h"

struct PlayerComponent {
	PlayerComponent() {
		score=0.;
		isReadyToStart = false;
	}
	int score;
	bool isReadyToStart;
};

#define thePlayerSystem PlayerSystem::GetInstance()
#define PLAYER(e) thePlayerSystem.Get(e)

UPDATABLE_SYSTEM(Player)
};
