#include "HUDManager.h"
#include <iostream>
class HUDManager::HUDManagerData {
	public:
		HUDManagerData() {
				time = 0;
				score = 0;
				niveau = 1;
				multiplier = 1;
		}
		float time,score, multiplier;
		int niveau;
};

void HUDManager::Setup() {
	this->datas = new HUDManagerData();
}

void HUDManager::ScoreCalc(int nb) {
	datas->score += datas->niveau*nb*datas->multiplier;
}

void HUDManager::Update(float dt) {
	datas->time += dt;
	//std::cout << datas->score << std::endl;
}

