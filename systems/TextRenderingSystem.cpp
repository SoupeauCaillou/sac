#include "TextRenderingSystem.h"
#include "../base/MathUtil.h"

#include "RenderingSystem.h"
#include "TransformationSystem.h"
#include "base/EntityManager.h"


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
					rc->color = trc->color;
					if (!trc->alignL) TRANSFORM(trc->drawing[i])->position = Vector2(i*trc->charSize.X-MathUtil::Min(trc->text.length(),trc->drawing.size())*trc->charSize.X, 0);
					else TRANSFORM(trc->drawing[i])->position = Vector2(i*trc->charSize.X, 0);

				}
			}
		}
		for(int i=trc->text.length(); i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->hide = trc->hide;
			RENDERING(trc->drawing[i])->texture = -1;
		}
	}
}



Entity TextRenderingSystem::CreateLocalEntity(int maxSymbol)
{
	int columnCount = 13;
	int rowCount = 8;
	std::map<char, Vector2> char2UV;

	for (int row=0; row<rowCount; row++)
		for (int column=0; column<columnCount; column++)
			char2UV[' '+ columnCount*row + column] = Vector2(column / (float)columnCount, row / (float)rowCount);

	Entity eTime = theEntityManager.CreateEntity();
	ADD_COMPONENT(eTime, Transformation);
	ADD_COMPONENT(eTime, TextRendering);
	TEXT_RENDERING(eTime)->fontBitmap = theRenderingSystem.loadTextureFile("figures.png");
	TEXT_RENDERING(eTime)->uvSize = Vector2(1. / columnCount, 1. / rowCount);
	TEXT_RENDERING(eTime)->charSize = Vector2(0.5, 1);
	TEXT_RENDERING(eTime)->char2UV = char2UV;
	// max time size maxSymbol numbers
	for(int i=0; i<maxSymbol; i++) {
		Entity e = theEntityManager.CreateEntity();
		ADD_COMPONENT(e, Transformation);
		TRANSFORM(e)->parent = eTime;
		ADD_COMPONENT(e, Rendering);
		TEXT_RENDERING(eTime)->drawing.push_back(e);
	}			
	return eTime;
}



