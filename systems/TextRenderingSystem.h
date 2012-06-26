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

	TextRenderingComponent() : text(""), fontName("typo"), positioning(CENTER), hide(false), isANumber(false), caretShown(false), caretSpeed(0), caretDt(0) {}
	std::string text;
	Color color;
	TextureRef fontBitmap;
	std::map<char, Vector2> char2UV;
	Vector2 uvSize;
	float charHeight;
	std::string fontName;
	float positioning;
	bool hide, isANumber, caretShown;
	float caretSpeed, caretDt;
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
	std::list<Entity> renderingEntitiesPool;
	std::map<std::string, std::map<unsigned char, float> > fontRegistry;
};
