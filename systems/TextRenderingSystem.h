#pragma once

#ifdef ANDROID
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#include <map>

#include "base/Vector2.h"
#include "base/EntityManager.h"
#include "base/MathUtil.h"

#include "TransformationSystem.h"
#include "System.h"
#include "RenderingSystem.h"

struct TextRenderingComponent {
    const static float LEFT;
    const static float CENTER;
    const static float RIGHT;

	TextRenderingComponent() : text(""), fontName("typo"), positioning(CENTER), hide(false), isANumber(false) {}
	std::string text;
	Color color;
	TextureRef fontBitmap;
	std::map<char, Vector2> char2UV;
	Vector2 uvSize;
	float charHeight;
	std::string fontName;
	float positioning;
	bool hide, isANumber;
	// managed by systems
	std::vector<Entity> drawing;
};

#define theTextRenderingSystem TextRenderingSystem::GetInstance()
#define TEXT_RENDERING(e) theTextRenderingSystem.Get(e)

UPDATABLE_SYSTEM(TextRendering)

public :
	Entity CreateEntity();
	void DeleteEntity(Entity e);

	void registerFont(const std::string& fontName, const std::map<unsigned char, float>& charH2Wratio) {
		fontRegistry[fontName] = charH2Wratio;
	}

private:
	std::list<Entity> renderingEntitiesPool;
	std::map<std::string, std::map<unsigned char, float> > fontRegistry;
};
