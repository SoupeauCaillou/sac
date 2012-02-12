#include "HUDManager.h"
#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include <iostream>
#include <sstream>
class HUDManager::HUDManagerData {
	public:
		HUDManagerData() {
				time = 0;
				score = 0;
				niveau = 1;
				multiplier = 1;

				eScore = 10000; // EntityManager.CreateEntity..
				theTransformationSystem.Add(eScore);
				TRANSFORM(eScore)->position = Vector2(1, 1);
				theTextRenderingSystem.Add(eScore);
				TEXT_RENDERING(eScore)->fontBitmap = theRenderingSystem.loadTextureFile("figures.png");
				TEXT_RENDERING(eScore)->uvSize = Vector2(0.1, 1);
				TEXT_RENDERING(eScore)->charSize = Vector2(1, 1);
				std::map<char, Vector2> char2UV;
				char2UV['0'] = Vector2(0, 0);
				char2UV['1'] = Vector2(0.1, 0);
				char2UV['2'] = Vector2(0.2, 0);
				char2UV['3'] = Vector2(0.3, 0);
				char2UV['4'] = Vector2(0.4, 0);
				char2UV['5'] = Vector2(0.5, 0);
				char2UV['6'] = Vector2(0.6, 0);
				char2UV['7'] = Vector2(0.7, 0);
				char2UV['8'] = Vector2(0.8, 0);
				char2UV['9'] = Vector2(0.9, 0);
				TEXT_RENDERING(eScore)->char2UV = char2UV;
				// max score size 10 numbers
				for(int i=0; i<10; i++) {
					Entity e = 10000 + i;
					theTransformationSystem.Add(e);
					theRenderingSystem.Add(e);
					TEXT_RENDERING(eScore)->drawing.push_back(e);
				}
				
		}
		float time,score, multiplier;
		int niveau;

		Entity eScore;
};

void HUDManager::Setup() {
	this->datas = new HUDManagerData();
}

void HUDManager::ScoreCalc(int nb) {
	datas->score += datas->niveau*nb*datas->multiplier;
}

void HUDManager::Update(float dt) {
	datas->time += dt;

	std::stringstream a;
	a.precision(0);
	a << std::fixed << datas->score;
	TEXT_RENDERING(datas->eScore)->text = a.str();	
	//std::cout << datas->score << std::endl;
}

