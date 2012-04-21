#include "TextRenderingSystem.h"
#include <ctype.h>
#include <sstream>

INSTANCE_IMPL(TextRenderingSystem);

TextRenderingSystem::TextRenderingSystem() : ComponentSystemImpl<TextRenderingComponent>("textrendering") {

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
		float spaceW = charH2Wratio['.'] * trc->charHeight;
		width += ((int) (trc->text.length() - 1) / 3) * spaceW;
	}
	for (int i=0; i<trc->text.length(); i++) {
		width += charH2Wratio[trc->text[i]] * trc->charHeight;
	}
	return width;
}

static float computeStartX(TextRenderingComponent* trc, std::map<unsigned char, float>& charH2Wratio) {
	switch (trc->positioning) {	
		case TextRenderingComponent::LEFT:
			return charH2Wratio[trc->text[0]] * trc->charHeight * 0.5;
		case TextRenderingComponent::RIGHT:
			return -computeStringWidth(trc, charH2Wratio) + charH2Wratio[trc->text[trc->text.length() - 1]] * 0.5;
		case TextRenderingComponent::CENTER:
		default:
			return -(computeStringWidth(trc, charH2Wratio) - charH2Wratio[trc->text[0]]) * 0.5;
	}
}

void TextRenderingSystem::DoUpdate(float dt) {
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		TextRenderingComponent* trc = (*it).second;
		TransformationComponent* trans = TRANSFORM(it->first);
		trans->size = Vector2::Zero;
		
		const int length = trc->text.length();
		std::map<unsigned char, float>& charH2Wratio = fontRegistry[trc->fontName];
		float x = computeStartX(trc, charH2Wratio);
		
		for(int i=0; i<length; i++) {
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
			a << (int)trc->text[i] << "_" << trc->fontName << ".png";
				
			if (trc->text[i] == ' ') {
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
				x += tc->size.X * 0.5;
			}
		}
		for(int i=trc->text.length(); i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->hide = true;
			renderingEntitiesPool.push_back(trc->drawing[i]);
		}
		trans->size = Vector2::Zero;
		trc->drawing.resize(trc->text.length());
	}
}



Entity TextRenderingSystem::CreateLocalEntity(int maxSymbol)
{
	Entity eTime = theEntityManager.CreateEntity();
	ADD_COMPONENT(eTime, Transformation);
	ADD_COMPONENT(eTime, TextRendering);
	TEXT_RENDERING(eTime)->charHeight = 0.5;
	TEXT_RENDERING(eTime)->fontName = "typo";
	return eTime;
}

void TextRenderingSystem::DestroyLocalEntity(Entity e) {
	TextRenderingComponent* tc = TEXT_RENDERING(e);
	if (!tc)
		return;
	for (int i=0; i<tc->drawing.size(); i++) {
		renderingEntitiesPool.push_back(tc->drawing[i]);
	}
	tc->drawing.clear();
	theEntityManager.DeleteEntity(e);
}
