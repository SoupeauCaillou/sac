#include "HUDManager.h"
#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

#include <iostream>
#include <sstream>

//FCRR : FPS Calculation Refresh Rate
#define FCRR 1.
class HUDManager::HUDManagerData {
	public:
		HUDManagerData() {
				frames = 0;
				nextfps = FCRR;	
				fps = 60;
		}
		Entity eScore, eTime, eLevel, eFPS, eObj[8],fBonus;
		int frames;
		float nextfps, fps;
};

void HUDManager::Setup() {
	this->datas = new HUDManagerData();
	
	datas->eScore = theTextRenderingSystem.CreateLocalEntity(10);
	datas->eTime = theTextRenderingSystem.CreateLocalEntity(10);
	datas->eFPS = theTextRenderingSystem.CreateLocalEntity(10);
	datas->eLevel = theTextRenderingSystem.CreateLocalEntity(10);

	TRANSFORM(datas->eLevel)->position = Vector2(5, 8);
	TRANSFORM(datas->eScore)->position = Vector2(5, 7);
	TRANSFORM(datas->eTime)->position = Vector2(0, 7);
	TRANSFORM(datas->eFPS)->position = Vector2(-2.5, 8);

	for (int i=0;i<8;i++) {
		datas->eObj[i] = theTextRenderingSystem.CreateLocalEntity(5);
		TRANSFORM(datas->eObj[i])->position = Vector2(i-3.5,6);
		TEXT_RENDERING(datas->eObj[i])->charSize /= 2;
	}

	datas->fBonus = theEntityManager.CreateEntity();
	
	
	ADD_COMPONENT(datas->fBonus, Transformation);
	ADD_COMPONENT(datas->fBonus, Rendering);

	RENDERING(datas->fBonus)->size = Vector2(2,2);
	TRANSFORM(datas->fBonus)->position = Vector2(2,7.5);
	TRANSFORM(datas->fBonus)->rotation = -.8;


	TEXT_RENDERING(datas->eFPS)->charSize /= 2;
	TEXT_RENDERING(datas->eFPS)->color = Color(0.1, 0.5, 0.4);
	
	Hide(true);
}

void HUDManager::Hide(bool toHide) {
	TEXT_RENDERING(datas->eScore)->hide = toHide;
	TEXT_RENDERING(datas->eTime)->hide = toHide;
	TEXT_RENDERING(datas->eFPS)->hide = toHide;
	TEXT_RENDERING(datas->eLevel)->hide = toHide;
	RENDERING(datas->fBonus)->hide = toHide;

	for (int i=0;i<8;i++) TEXT_RENDERING(datas->eObj[i])->hide = toHide;
}


void HUDManager::Update(float dt) {
	//Score
	{
	std::stringstream a;
	a.precision(0);
	a << std::fixed << thePlayerSystem.GetScore();
	TEXT_RENDERING(datas->eScore)->text = a.str();	
	}
	//Temps
	{
	std::stringstream a;
	int time = TIMELIMIT-thePlayerSystem.GetTime();
	int minute = time/60;
	int seconde= time%60;
	// faudrait que a soit de la forme xx:xx s, meme 01:03 s
	a << minute << ":" << seconde << " s";
	TEXT_RENDERING(datas->eTime)->text = a.str();
	}
	//FPS
	{
	std::stringstream a;
	datas->nextfps-=dt;
	datas->frames++;

	if (datas->nextfps<0) {
		datas->fps = datas->frames/FCRR;
		datas->nextfps = FCRR;
		datas->frames = 0;
	}
	a << "FPS : " << datas->fps;
	TEXT_RENDERING(datas->eFPS)->text = a.str();
	}

	//Level
	{
	std::stringstream a;
	a << thePlayerSystem.GetLevel();
	TEXT_RENDERING(datas->eLevel)->text = a.str();
	}
	//Objectifs
	for (int i=0;i<8;i++)
	{
	std::stringstream a;
	a << thePlayerSystem.GetRemain(i) << "/" << thePlayerSystem.GetObj();
	TEXT_RENDERING(datas->eObj[i])->text = a.str();
	}
	//Feuille Bonus
	{
	std::stringstream a;
	a <<thePlayerSystem.GetBonus()<<".png";	
	RENDERING(datas->fBonus)->texture = theRenderingSystem.loadTextureFile(a.str());
	}
}

