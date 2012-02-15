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
		Entity eScore, eTime, eFPS;
		int frames;
		float nextfps, fps;
};

void HUDManager::Setup() {
	this->datas = new HUDManagerData();
	
	datas->eScore = theTextRenderingSystem.CreateLocalEntity(10);
	datas->eTime = theTextRenderingSystem.CreateLocalEntity(10);
	datas->eFPS = theTextRenderingSystem.CreateLocalEntity(10);		
				
	TRANSFORM(datas->eScore)->position = Vector2(5, 6);
	TRANSFORM(datas->eTime)->position = Vector2(0, 7.5);
	TRANSFORM(datas->eFPS)->position = Vector2(5, 8);

}

void HUDManager::Hide(bool toHide) {
	TEXT_RENDERING(datas->eScore)->hide = toHide;	
	TEXT_RENDERING(datas->eTime)->hide = toHide;	
	TEXT_RENDERING(datas->eFPS)->hide = toHide;	
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
	int time = 10-thePlayerSystem.GetTime();
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
	a << "sss : " << datas->fps;
	TEXT_RENDERING(datas->eFPS)->text = a.str();	
	}
}

