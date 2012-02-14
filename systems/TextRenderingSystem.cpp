#include "TextRenderingSystem.h"
#include "../base/MathUtil.h"

#include "RenderingSystem.h"
#include "TransformationSystem.h"

INSTANCE_IMPL(TextRenderingSystem);

TextRenderingSystem::TextRenderingSystem() : ComponentSystemImpl<TextRenderingComponent>("textrendering") { 
	
}


void TextRenderingSystem::DoUpdate(float dt) {
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		TextRenderingComponent* trc = (*it).second;
		for(int i=0; i<trc->text.length() && i < trc->drawing.size(); i++) {
			RenderingComponent* rc = RENDERING(trc->drawing[i]);
			rc->hide = trc->hide;

			if (!rc->hide) {
				std::map<char, Vector2>::iterator jt = trc->char2UV.find(trc->text[i]);
				if (trc->text[i] != ' ' && jt == trc->char2UV.end()) {
					std::cout << "Char '" << trc->text[i] << "'" << " not found in font bitmap" << std::endl; 
					rc->texture = -1;
				} else {
					if (trc->text[i] == ' ') {
						rc->texture = -1;
					} else {
						rc->texture = trc->fontBitmap;
						rc->bottomLeftUV = jt->second;
						rc->topRightUV = jt->second + trc->uvSize;
					}
					rc->size = trc->charSize;
					TRANSFORM(trc->drawing[i])->position = Vector2(i*trc->charSize.X-trc->text.length(), 0);
				}
			}
		}
		for(int i=trc->text.length(); i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->hide = trc->hide;
			RENDERING(trc->drawing[i])->texture = -1;
		}
	}
}

