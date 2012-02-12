#include "TextRenderingSystem.h"
#include "../base/MathUtil.h"

#include "RenderingSystem.h"
#include "TransformationSystem.h"

INSTANCE_IMPL(TextRenderingSystem);

TextRenderingSystem::TextRenderingSystem() : ComponentSystem<TextRenderingComponent>("textrendering") { 
	
}


void TextRenderingSystem::DoUpdate(float dt) {
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		TextRenderingComponent* trc = (*it).second;
		TransformationComponent* tf = TRANSFORM((*it).first);
		for(int i=0; i<trc->text.length() && i < trc->drawing.size(); i++) {
			RenderingComponent* rc = RENDERING(trc->drawing[i]);
			std::map<char, Vector2>::iterator jt = trc->char2UV.find(trc->text[i]);
			if (jt == trc->char2UV.end()) {
				std::cout << "Char '" << trc->text[i] << "'" << " not found in font bitmap" << std::endl; 
				rc->texture = -1;
			} else {
				rc->texture = trc->fontBitmap;
				rc->bottomLeftUV = jt->second;
				rc->topRightUV = jt->second + trc->uvSize;
				rc->size = trc->charSize;
				TRANSFORM(trc->drawing[i])->worldPosition = tf->worldPosition + Vector2(i*trc->charSize.X, 0);
			}
		}
		for(int i=trc->text.length(); i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->texture = -1;
		}
	}
}

