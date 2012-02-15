#pragma once

#include "System.h"
#include "base/EntityManager.h"
#include <iostream>

struct PlayerComponent {
	PlayerComponent() {
		score=0;
		isReadyToStart = false;
		time = 0.;
		level = 1;
		multiplier = 1.;
	}
	int score, level;
	float time, multiplier;
	bool isReadyToStart;
};


#define thePlayerSystem PlayerSystem::GetInstance()
#define PLAYER(e) thePlayerSystem.Get(e)

UPDATABLE_SYSTEM(Player)

public : 

	float GetTime();
	
	int GetScore();
	 
	void ScoreCalc(int nb);
	
	void Reset();
};
