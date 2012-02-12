#pragma once
#include "System.h"

class HUDManager {
	public:
			
		void Setup();
		
		void Update(float dt);
		
		void ScoreCalc(int nb);

	private:
		class HUDManagerData;
		HUDManagerData* datas;
};
