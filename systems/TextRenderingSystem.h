#pragma once

#if defined(ANDROID) || defined(EMSCRIPTEN)
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#include "System.h"

#include <list>

struct TextRenderingComponent {
	const static float LEFT;
	const static float CENTER;
	const static float RIGHT;

	const static int IsANumberBit = 1 << 0;
	const static int AdjustHeightToFillWidthBit = 1 << 1;
	const static int MultiLineBit = 1 << 2;

	TextRenderingComponent() : text(""), fontName("typo"),
	positioning(CENTER), hide(true), flags(0), cameraBitMask(~0U) {
		caret.show = false;
		caret.speed = caret.dt = 0;
		blink.offDuration =
		blink.onDuration =
		blink.accum = 0;
	}

	std::string text;
	Color color;

	//if its localizable
	std::string localizableID;

	union {
		float charHeight;
		float maxCharHeight;
	};

	std::string fontName;
	float positioning;
	bool hide;
	int flags;
	struct {
		bool show;
		float speed;
		float dt;
	} caret;
	struct {
		float offDuration;
		float onDuration;
		float accum;
	} blink;
	unsigned cameraBitMask;

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

	float computeTextRenderingComponentWidth(TextRenderingComponent* trc);

private:
    std::map<Entity, unsigned int> cache;
	std::list<Entity> renderingEntitiesPool;
	std::map<std::string, std::map<unsigned char, float> > fontRegistry; // UTF8 8->32 bits : utiliser un unsigned int
};
