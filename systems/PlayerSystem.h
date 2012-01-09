#pragma once

#include "System.h"
#include "stdafx.h"

struct PlayerComponent {
	PlayerComponent() {
		score=0.;
		isReadyToStart = false;
	}
	int score;
	bool isReadyToStart;
	
	Vector2 directionTarget;
};

#define thePlayerSystem PlayerSystem::GetInstance()
#define PLAYER(e) thePlayerSystem.Get(e)

UPDATABLE_SYSTEM(Player)

//public:
	//void ReceiveMessage(Message* msg);

};
