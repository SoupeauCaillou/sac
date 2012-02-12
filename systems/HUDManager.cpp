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


				std::map<char, Vector2> char2UV;
				char2UV['0'] = Vector2(0, 0);
				char2UV['1'] = Vector2(1/16., 0);
				char2UV['2'] = Vector2(2/16., 0);
				char2UV['3'] = Vector2(3/16., 0);
				char2UV['4'] = Vector2(4/16., 0);
				char2UV['5'] = Vector2(5/16., 0);
				char2UV['6'] = Vector2(6/16., 0);
				char2UV['7'] = Vector2(7/16., 0);
				char2UV['8'] = Vector2(8/16., 0);
				char2UV['9'] = Vector2(9/16., 0);
				char2UV[':'] = Vector2(10/16., 0);
				char2UV['.'] = Vector2(11/16., 0);
								
				
				
				eScore = 10000; // EntityManager.CreateEntity..
				theTransformationSystem.Add(eScore);
				theTextRenderingSystem.Add(eScore);
				TEXT_RENDERING(eScore)->fontBitmap = theRenderingSystem.loadTextureFile("figures.png");
				TEXT_RENDERING(eScore)->uvSize = Vector2(0.0625, 1);
				TEXT_RENDERING(eScore)->charSize = Vector2(1, 1);
				TEXT_RENDERING(eScore)->char2UV = char2UV;
				// max score size 10 numbers
				for(int i=0; i<10; i++) {
					Entity e = 10000 + i;
					theTransformationSystem.Add(e);
					theRenderingSystem.Add(e);
					TEXT_RENDERING(eScore)->drawing.push_back(e);
				}
				
				eTime = 20000;
				theTransformationSystem.Add(eTime);
				theTextRenderingSystem.Add(eTime);
				TEXT_RENDERING(eTime)->fontBitmap = theRenderingSystem.loadTextureFile("figures.png");
				TEXT_RENDERING(eTime)->uvSize = Vector2(0.1, 1);
				TEXT_RENDERING(eTime)->charSize = Vector2(1, 1);
				TEXT_RENDERING(eTime)->char2UV = char2UV;
				// max time size 10 numbers
				for(int i=0; i<10; i++) {
					Entity e = 20000 + i;
					theTransformationSystem.Add(e);
					theRenderingSystem.Add(e);
					TEXT_RENDERING(eTime)->drawing.push_back(e);
				}				
				
		}
		float time,score, multiplier;
		int niveau;

		Entity eScore, eTime;
};

void HUDManager::Setup() {
	this->datas = new HUDManagerData();
	TRANSFORM(datas->eScore)->position = Vector2(5, 6);
	TRANSFORM(datas->eTime)->position = Vector2(5, 7.5);
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
	
	int minute = ((int)datas->time)/60;
	int seconde = ((int)datas->time)%60;
	a << minute << ":" << seconde;
	// std::cout << a.str() << std::endl;
	TEXT_RENDERING(datas->eTime)->text = a.str();	
}

