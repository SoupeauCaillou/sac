#pragma once

#include "System.h"
#include "../base/Vector2.h"
#include <GLES2/gl2.h>
#include <map>

#include "RenderingSystem.h"

struct TextRenderingComponent {
	std::string text;
	TextureRef fontBitmap;
	std::map<char, Vector2> char2UV;
	Vector2 uvSize;
	Vector2 charSize;

	std::vector<Entity> drawing;
};

#define theTextRenderingSystem TextRenderingSystem::GetInstance()
#define TEXT_RENDERING(e) theTextRenderingSystem.Get(e)

UPDATABLE_SYSTEM(TextRendering)
};
