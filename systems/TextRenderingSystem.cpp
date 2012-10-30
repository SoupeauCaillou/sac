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

const char InlineImageDelimiter = 0x97;

INSTANCE_IMPL(TextRenderingSystem);

TextRenderingSystem::TextRenderingSystem() : ComponentSystemImpl<TextRenderingComponent>("TextRendering") {
    /* nothing saved */
}


static Entity createRenderingEntity() {
	Entity e = theEntityManager.CreateEntity();
	ADD_COMPONENT(e, Transformation);
	ADD_COMPONENT(e, Rendering);
	return e;
}

static void parseInlineImageString(const std::string& s, std::string* image, float* w, float* h) {
    int idx0 = s.find(',');
    int idx1 = s.find(',', idx0 + 1);
    if (image)
        *image = s.substr(0, idx0);
    if (w)
        *w = atof(s.substr(idx0 + 1, idx1 - idx0).c_str());
    if (h)
        *h = atof(s.substr(idx1 + 1, s.length() - idx1).c_str());
}

static float computePartialStringWidth(TextRenderingComponent* trc, size_t from, size_t to, float charHeight, std::map<unsigned char, float>& charH2Wratio) {
	// assume monospace ...
	float width = 0;
	if (trc->flags & TextRenderingComponent::IsANumberBit) {
		float spaceW = charH2Wratio['a'] * charHeight * 0.75;
		width += ((int) (trc->text.length() - 1) / 3) * spaceW;
	}
	for (unsigned int i=from; i<to; i++) {
		char letter = trc->text[i];
		if (letter == (char)0xC3 || letter == (char)0xC2) {
            letter = trc->text[i+1];
            if (letter == InlineImageDelimiter) {
                // looks for next delimiter
                unsigned int end = trc->text.find(InlineImageDelimiter, i+2);
                float w = 0;
                parseInlineImageString(trc->text.substr(i+2, end - 1 - (i+2) + 1), 0, &w, 0);
                width += w * charHeight;
                i = end;
            }
        } else if (letter == '\n') {
            continue;
        } else {
			width += charH2Wratio[trc->text[i]] * charHeight;
		}
	}
	return width;
}

static float computeStringWidth(TextRenderingComponent* trc, float charHeight, std::map<unsigned char, float>& charH2Wratio) {
	return computePartialStringWidth(trc, 0, trc->text.length(), charHeight, charH2Wratio);
}

static float computeStartX(TextRenderingComponent* trc, float charHeight, std::map<unsigned char, float>& charH2Wratio) {
    float result = -computeStringWidth(trc, charHeight, charH2Wratio) * trc->positioning;

    return result;
}

float TextRenderingSystem::computeTextRenderingComponentWidth(TextRenderingComponent* trc) {
	std::map<unsigned char, float>& charH2Wratio = fontRegistry[trc->fontName];
	return computeStringWidth(trc, trc->charHeight, charH2Wratio);
}

void TextRenderingSystem::DoUpdate(float dt) {
	/* render */
    FOR_EACH_ENTITY_COMPONENT(TextRendering, entity, trc)
		// early quit if hidden
		if (trc->hide) {
			for (int i=0; i<trc->drawing.size(); i++) {
				RENDERING(trc->drawing[i])->hide = true;
				renderingEntitiesPool.push_back(trc->drawing[i]);
			}
			trc->drawing.clear();
			continue;
		}

		TransformationComponent* trans = TRANSFORM(entity);
		bool caret = false;
		// caret blink
		if (trc->caret.speed > 0) {
			trc->caret.dt += dt;
			if (trc->caret.dt > trc->caret.speed) {
				trc->caret.dt = 0;
				trc->caret.show = !trc->caret.show;
			}
			caret = true;
			trc->text.push_back('_');
		}

		const unsigned int length = trc->text.length();

		std::map<unsigned char, float>& charH2Wratio = fontRegistry[trc->fontName];
		
		float charHeight = trc->charHeight;
		if (trc->flags & TextRenderingComponent::AdjustHeightToFillWidthBit) {
			float targetWidth = trans->size.X;
			charHeight = targetWidth / computeStringWidth(trc, 1, charH2Wratio);
			if (trc->maxCharHeight > 0 ) {
				charHeight = MathUtil::Min(trc->maxCharHeight, charHeight);
			}
		}
		
		float x = (trc->flags & TextRenderingComponent::MultiLineBit) ? 
			(trans->size.X * -0.5) : computeStartX(trc, charHeight, charH2Wratio);
		float y = 0;
		const float startX = x;
		bool newWord = true;
        int count = 0;
		for(unsigned int i=0; i<length; i++) {
			// add sub-entity if needed
			if (count >= trc->drawing.size()) {
				if (renderingEntitiesPool.size() > 0) {
					trc->drawing.push_back(renderingEntitiesPool.back());
					renderingEntitiesPool.pop_back();					
				} else {
					trc->drawing.push_back(createRenderingEntity());
				}
			}

			RenderingComponent* rc = RENDERING(trc->drawing[count]);
			TransformationComponent* tc = TRANSFORM(trc->drawing[count]);
			tc->parent = entity;
            tc->worldPosition = trans->worldPosition;
			rc->hide = trc->hide;
            rc->cameraBitMask = trc->cameraBitMask;

			if (rc->hide)
				continue;
				
			if (trc->flags & TextRenderingComponent::MultiLineBit) {
				size_t wordEnd = trc->text.find_first_of(" ,:.", i);
				size_t lineEnd = trc->text.find_first_of("\n", i);
				bool newLine = false;
				if (wordEnd == i) {
					newWord = true;
				} else if (lineEnd == i) {
					newLine = true;
				} else if (newWord) {
					if (wordEnd == std::string::npos) {
						wordEnd = trc->text.length();
					}
					// compute length of next word. If it doesn't fit on current line
					// => go to next line
					
					float w = computePartialStringWidth(trc, i, wordEnd - 1, charHeight, charH2Wratio);
					if (x + w >= trans->size.X * 0.5) {
						newLine = true;
					}
					newWord = false;
				}
				
				if (newLine) {
					y -= 1.2 * charHeight;
					x = startX;
				}
			}
			std::stringstream a;
			char letter = trc->text[i];
			bool inlineImage = false;
            int skip = -1;
			// Unicode control caracter, skipping
			if (letter == (char)0xC3 || letter == (char)0xC2) {
				rc->hide = true;
				continue;
			} else {
				int l = (int) ((letter < 0) ? (unsigned char)letter : letter);
				if (letter == InlineImageDelimiter) {
                    std::string texture;
					inlineImage = true;
                    int next = trc->text.find(InlineImageDelimiter, i+1);
                    parseInlineImageString(
                        trc->text.substr(i+1, next - 1 - (i+1) + 1), &texture, &tc->size.X, &tc->size.Y);
                    a << texture;
                    tc->size.X *= charHeight;
                    tc->size.Y *= charHeight;
                    skip = next;
				} else {
					a << l << "_" << trc->fontName;
                    tc->size = Vector2(charHeight * charH2Wratio[trc->text[i]], charHeight);
				}
			}

			if (trc->text[i] == ' ' || (i==length-1 && trc->caret.speed > 0 && !trc->caret.show)) {
				rc->hide = true;
			} else {
				rc->texture = theRenderingSystem.loadTextureFile(a.str());
				if (!inlineImage) rc->color = trc->color;
				else rc->color = Color();
			}
            count++;
			x += tc->size.X * 0.5;
			tc->position = Vector2(x, y + (inlineImage ? tc->size.Y * 0.25 : 0));
			x += tc->size.X * 0.5;
 			if (trc->flags & TextRenderingComponent::IsANumberBit && ((length - i - 1) % 3) == 0) {
				x += charH2Wratio['a'] * charHeight * 0.75;
			}
         
            if (skip >= 0) {
                i = skip;
            }
		}
		for(unsigned int i=count; i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->hide = true;
			renderingEntitiesPool.push_back(trc->drawing[i]);
		}
		trc->drawing.resize(count);
		
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
        TRANSFORM(tc->drawing[i])->parent = 0;
		renderingEntitiesPool.push_back(tc->drawing[i]);
		RENDERING(tc->drawing[i])->hide = true;
	}
	tc->drawing.clear();
	theEntityManager.DeleteEntity(e);
}
