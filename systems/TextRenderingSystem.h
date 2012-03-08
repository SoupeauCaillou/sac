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
	TextRenderingComponent() : text(""), hide(false), alignL(false) {}
	std::string text;
	Color color;
	TextureRef fontBitmap;
	std::map<char, Vector2> char2UV;
	Vector2 uvSize;
	Vector2 charSize;
	bool hide, alignL;
	// managed by systems
	std::vector<Entity> drawing;
};

#define theTextRenderingSystem TextRenderingSystem::GetInstance()
#define TEXT_RENDERING(e) theTextRenderingSystem.Get(e)

UPDATABLE_SYSTEM(TextRendering)

public :
	Entity CreateLocalEntity(int maxSymbol);
	void DestroyLocalEntity(Entity e);

private:
	std::list<Entity> renderingEntitiesPool;
};
