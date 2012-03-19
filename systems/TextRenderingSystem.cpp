#include "TextRenderingSystem.h"



INSTANCE_IMPL(TextRenderingSystem);

TextRenderingSystem::TextRenderingSystem() : ComponentSystemImpl<TextRenderingComponent>("textrendering") {

}


Entity createRenderingEntity() {
	Entity e = theEntityManager.CreateEntity();
	ADD_COMPONENT(e, Transformation);
	ADD_COMPONENT(e, Rendering);
	return e;
}

void TextRenderingSystem::DoUpdate(float dt) {
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		TextRenderingComponent* trc = (*it).second;
		for(int i=0; i<trc->text.length(); i++) {
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
				
			std::map<char, Vector2>::iterator jt = trc->char2UV.find(trc->text[i]);
			if (trc->text[i] != ' ' && jt == trc->char2UV.end()) {
				std::cout << "Char '" << trc->text[i] << "'" << " not found in font bitmap" << std::endl;
				rc->texture = -1;
				rc->hide = true;
			} else {
				if (trc->text[i] == ' ') {
					rc->hide = true;
				} else {
					rc->texture = trc->fontBitmap;
					rc->bottomLeftUV = jt->second;
					rc->topRightUV = jt->second + trc->uvSize;
				}
				tc->size = trc->charSize;
				rc->color = trc->color;
				if (!trc->alignL) {
					tc->position = Vector2(i*trc->charSize.X-MathUtil::Min(trc->text.length(),trc->drawing.size())*trc->charSize.X, 0);
				} else {
					tc->position = Vector2(i*trc->charSize.X, 0);
				}
			}
		}
		for(int i=trc->text.length(); i < trc->drawing.size(); i++) {
			RENDERING(trc->drawing[i])->texture = -1;
			renderingEntitiesPool.push_back(trc->drawing[i]);
		}
		trc->drawing.resize(trc->text.length());
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
