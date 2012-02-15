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
					TRANSFORM(trc->drawing[i])->position = Vector2(i*trc->charSize.X-MathUtil::Min(trc->text.length(),trc->drawing.size())*trc->charSize.X, 0);
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
	int count = 13;
	std::map<char, Vector2> char2UV;
	char2UV['0'] = Vector2(0 / (float)count, 0);
	char2UV['1'] = Vector2(1 / (float)count, 0);
	char2UV['2'] = Vector2(2 / (float)count, 0);
	char2UV['3'] = Vector2(3 / (float)count, 0);
	char2UV['4'] = Vector2(4 / (float)count, 0);
	char2UV['5'] = Vector2(5 / (float)count, 0);
	char2UV['6'] = Vector2(6 / (float)count, 0);
	char2UV['7'] = Vector2(7 / (float)count, 0);
	char2UV['8'] = Vector2(8 / (float)count, 0);
	char2UV['9'] = Vector2(9 / (float)count, 0);
	char2UV[':'] = Vector2(10 / (float)count, 0);
	char2UV['.'] = Vector2(11 / (float)count, 0);
	char2UV['s'] = Vector2(12 / (float)count, 0);
	
	Entity eTime = theEntityManager.CreateEntity();
	theTransformationSystem.Add(eTime);
	theTextRenderingSystem.Add(eTime);
	TEXT_RENDERING(eTime)->fontBitmap = theRenderingSystem.loadTextureFile("figures.png");
	TEXT_RENDERING(eTime)->uvSize = Vector2(1.0 / count, 1);
	TEXT_RENDERING(eTime)->charSize = Vector2(0.5, 1);
	TEXT_RENDERING(eTime)->char2UV = char2UV;
	// max time size maxSymbol numbers
	for(int i=0; i<maxSymbol; i++) {
		Entity e = theEntityManager.CreateEntity();
		theTransformationSystem.Add(e);
		TRANSFORM(e)->parent = eTime;
		theRenderingSystem.Add(e);
		TEXT_RENDERING(eTime)->drawing.push_back(e);
	}			
	return eTime;
}

