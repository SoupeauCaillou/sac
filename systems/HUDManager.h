#pragma once
#include "System.h"
#include "PlayerSystem.h"

class HUDManager {
	public:
			
		void Setup();
		
		void Update(float dt);
		
		void ScoreCalc(int nb);

		void Hide(bool toHide);
		
	private:
		class HUDManagerData;
		HUDManagerData* datas;
};
