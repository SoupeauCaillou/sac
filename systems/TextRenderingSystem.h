/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#if defined(ANDROID) || defined(EMSCRIPTEN)
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
    
    const static int IsANumberBit = 1 << 0;
    const static int AdjustHeightToFillWidthBit = 1 << 1;
	const static int MultiLineBit = 1 << 2;
	
	TextRenderingComponent() : 
		text(""), 
		fontName("typo"), 
		positioning(CENTER), 
		hide(false), 
		flags(0),
        cameraBitMask(~0U) {
			caret.show = false;
			caret.speed = caret.dt = 0;
		}
	std::string text;
	Color color;
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
	std::map<std::string, std::map<unsigned char, float> > fontRegistry;
};
