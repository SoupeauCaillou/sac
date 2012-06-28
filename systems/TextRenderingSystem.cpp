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
#include "TextRenderingSystem.h"
#include <ctype.h>
#include <sstream>

const float TextRenderingComponent::LEFT = 0.0f;
const float TextRenderingComponent::CENTER = 0.5f;
const float TextRenderingComponent::RIGHT = 1.0f;


INSTANCE_IMPL(TextRenderingSystem);

TextRenderingSystem::TextRenderingSystem() : ComponentSystemImpl<TextRenderingComponent>("TextRendering") {

}


Entity createRenderingEntity() {
	Entity e = theEntityManager.CreateEntity();
	ADD_COMPONENT(e, Transformation);
	ADD_COMPONENT(e, Rendering);
	return e;
}

static float computeStringWidth(TextRenderingComponent* trc, std::map<unsigned char, float>& charH2Wratio) {
	// assume monospace ...
	float width = 0;
	if (trc->isANumber) {
		float spaceW = charH2Wratio['a'] * trc->charHeight * 0.75;
		width += ((int) (trc->text.length() - 1) / 3) * spaceW;
	}
	for (unsigned int i=0; i<trc->text.length(); i++) {
		char letter = trc->text[i];
		if (letter != (char)0xC3) {
			width += charH2Wratio[trc->text[i]] * trc->charHeight;
		}
	}
	return width;
}

static float computeStartX(TextRenderingComponent* trc, std::map<unsigned char, float>& charH2Wratio) {
    float result = -computeStringWidth(trc, charH2Wratio) * trc->positioning;

    return result;
}

float TextRenderingSystem::computeTextRenderingComponentWidth(TextRenderingComponent* trc) {
	std::map<unsigned char, float>& charH2Wratio = fontRegistry[trc->fontName];
	return computeStringWidth(trc, charH2Wratio);
}

void TextRenderingSystem::DoUpdate(float dt) {
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		TextRenderingComponent* trc = (*it).second;
		TransformationComponent* trans = TRANSFORM(it->first);
		bool caret = false;
		trans->size = Vector2::Zero;

		if (trc->caretSpeed > 0) {
			trc->caretDt += dt;
			if (trc->caretDt > trc->caretSpeed) {
				trc->caretDt = 0;
				trc->caretShown = !trc->caretShown;
			}
			caret = true;
			trc->text.push_back('_');
		}

		const unsigned int length = trc->text.length();

		std::map<unsigned char, float>& charH2Wratio = fontRegistry[trc->fontName];
		float x = computeStartX(trc, charH2Wratio);

		for(unsigned int i=0; i<length; i++) {
			// add sub-entity if needed
			if (i >= trc->drawing.size()) {
				if (renderingEntitiesPool.size() > 0) {
					trc->drawing.push_back(renderingEntitiesPool.back());
					renderingEntitiesPool.pop_back();					
				} else {
					trc->drawing.push_back(createRenderingEntity());
				}
			}

			RenderingComponent* rc = RENDERING(trc->drawing[i]);
			TransformationComponent* tc = TRANSFORM(trc->drawing[i]);
			tc->parent = it->first;
			rc->hide = trc->hide;

			if (rc->hide)
				continue;

			std::stringstream a;
			char letter = trc->text[i];
			// Unicode control caracter, skipping
			if (letter == (char)0xC3) {
				rc->hide = true;
				continue;
			} else {
				a << (int) ((letter < 0) ? (unsigned char)letter : letter) << "_" << trc->fontName;
			}

			if (trc->text[i] == ' ' || (i==length-1 && trc->caretSpeed > 0 && !trc->caretShown)) {
				rc->hide = true;
			} else {
				rc->texture = theRenderingSystem.loadTextureFile(a.str());
				rc->color = trc->color;
			}
			tc->size = Vector2(trc->charHeight * charH2Wratio[trc->text[i]], trc->charHeight);
			x += tc->size.X * 0.5;
			tc->position = Vector2(x, 0);
			x += tc->size.X * 0.5;
 			if (trc->isANumber && ((length - i - 1) % 3) == 0) {
				x += charH2Wratio['a'] * trc->charHeight * 0.75;
			}
		}
		for(unsigned int i=trc->text.length(); i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->hide = true;
			renderingEntitiesPool.push_back(trc->drawing[i]);
		}
		trans->size = Vector2::Zero;
		trc->drawing.resize(trc->text.length());
		
		if (caret) {
			trc->text.resize(trc->text.length() - 1);	
		}
	}
}



Entity TextRenderingSystem::CreateEntity()
{
	Entity eTime = theEntityManager.CreateEntity();
	ADD_COMPONENT(eTime, Transformation);
	ADD_COMPONENT(eTime, TextRendering);
	TEXT_RENDERING(eTime)->charHeight = 0.5;
	TEXT_RENDERING(eTime)->fontName = "typo";
	return eTime;
}

void TextRenderingSystem::DeleteEntity(Entity e) {
	TextRenderingComponent* tc = TEXT_RENDERING(e);
	if (!tc)
		return;
	for (unsigned int i=0; i<tc->drawing.size(); i++) {
		renderingEntitiesPool.push_back(tc->drawing[i]);
		RENDERING(tc->drawing[i])->hide = true;
	}
	tc->drawing.clear();
	theEntityManager.DeleteEntity(e);
}
