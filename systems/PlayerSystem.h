#pragma once

#include "System.h"
#include "base/EntityManager.h"
#include <iostream>
#include "base/MathUtil.h"

struct PlayerComponent {
	PlayerComponent() {
		score=0;
		isReadyToStart = false;
		time = 0.;
		level = 1;
		bonus = MathUtil::RandomInt(8)+1;
		for (int i=0;i<50;i++)
			obj[i]=3+i;
			
		for (int i=0; i<8;i++)
			remain[i]=obj[0];
	}
	int score, level, obj[50], remain[8], bonus;
	float time;
	bool isReadyToStart;
};


#define thePlayerSystem PlayerSystem::GetInstance()
#define PLAYER(e) thePlayerSystem.Get(e)

UPDATABLE_SYSTEM(Player)

public : 
	void SetTime(float t);

	float GetTime();

	int GetScore();

	int GetBonus();
	
	void ScoreCalc(int nb, int type);

	void Reset();

	int GetRemain(int type);

	int GetObj();

	void LevelUp();

	int GetLevel();
};
