#pragma once

#include <GLES2/gl2.h>
#include <map>

#include "base/Vector2.h"
#include "base/EntityManager.h"
#include "base/MathUtil.h"

#include "TransformationSystem.h"
#include "System.h"
#include "RenderingSystem.h"

struct TextRenderingComponent {
	TextRenderingComponent() : text(""), hide(false), positioning(CENTER), isANumber(false) {}
	std::string text;
	Color color;
	TextureRef fontBitmap;
	std::map<char, Vector2> char2UV;
	Vector2 uvSize;
	float charHeight;
	std::string fontName;
	enum Positionning {
		CENTER,
		LEFT,
		RIGHT
	} positioning;
	bool hide, isANumber;
	// managed by systems
	std::vector<Entity> drawing;
};

#define theTextRenderingSystem TextRenderingSystem::GetInstance()
#define TEXT_RENDERING(e) theTextRenderingSystem.Get(e)

UPDATABLE_SYSTEM(TextRendering)

public :
	Entity CreateLocalEntity(int maxSymbol);
	void DestroyLocalEntity(Entity e);
	
	void registerFont(const std::string& fontName, const std::map<unsigned char, float>& charH2Wratio) {
		fontRegistry[fontName] = charH2Wratio;
	}

private:
	std::list<Entity> renderingEntitiesPool;
	std::map<std::string, std::map<unsigned char, float> > fontRegistry;
};
