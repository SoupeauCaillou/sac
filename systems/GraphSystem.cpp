#include "GraphSystem.h"

#include "base/EntityManager.h"

#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"

#define SIZE 256

static void putPixel(ImageDesc &textureDesc, int pos_x, int pos_y, unsigned char value=0xFF) {
	if ( pos_x > -1 &&  pos_x < textureDesc.width && pos_y > -1 &&  pos_y < textureDesc.height) {
        memset(textureDesc.datas + (pos_x + (textureDesc.width * (textureDesc.height-1 - pos_y))) * textureDesc.channels, value, 4);
    }
}

static void putPoint(ImageDesc &textureDesc, int pos_x, int pos_y, int lineWidth=1) {
	if (lineWidth == 0)
		lineWidth = 2;
    for (int i=-lineWidth / 2; i < lineWidth / 2; ++i) {
        for (int j=-lineWidth / 2; j < lineWidth / 2; ++j) {
			putPixel(textureDesc, pos_x + i, pos_y + j);
			//~ if ( pos_x+i > -1 &&  pos_x+i < textureDesc.width && pos_y+j > -1 &&  pos_y+j < textureDesc.height) {
				//~ textureDesc.datas[textureDesc.channels * (pos_x+i + (textureDesc.height-1 -(pos_y+j))*textureDesc.width) + 3] = ((i==j || i==-j) && (i == -lineWidth / 2 || i == lineWidth / 2 - 1)) ? 0x80 : 0xFF;
			//~ }
		}
    }    
}

INSTANCE_IMPL(GraphSystem);

GraphSystem::GraphSystem() : ComponentSystemImpl<GraphComponent>("Graph") {
    
}

void GraphSystem::DoUpdate(float dt) {
	for (std::map<TextureRef, ImageDesc>::iterator it=textureRef2Image.begin(); it != textureRef2Image.end(); ++it) {
		memset(it->second.datas, 0x40, it->second.width * it->second.height * it->second.channels);
	}
    FOR_EACH_ENTITY_COMPONENT(Graph, entity, gc)
        TextureRef texture = theRenderingSystem.textureLibrary.load(gc->textureName);

        std::map<TextureRef, ImageDesc>::iterator jt;
        if ( (jt = textureRef2Image.find(texture)) != textureRef2Image.end()) {
            drawTexture(jt->second, gc);
        }
        else {
            ImageDesc desc;
            desc.width = desc.height = SIZE;
            desc.channels = 4;
            desc.mipmap = 0;
            desc.type = ImageDesc::RAW;
            desc.datas = new char[desc.width * desc.height * desc.channels];

            drawTexture(desc, gc);

            theRenderingSystem.textureLibrary.registerDataSource(texture, desc);

            textureRef2Image.insert(std::make_pair(texture, desc));
        }
        theRenderingSystem.textureLibrary.reload(gc->textureName);
    }
}

void GraphSystem::drawTexture(ImageDesc &textureDesc, GraphComponent *gc) {
    
    float minScaleX = gc->pointsList.begin()->first, minScaleY = gc->pointsList.begin()->second, maxScaleX = gc->pointsList.begin()->first, maxScaleY = gc->pointsList.begin()->second;
    for (std::list<std::pair<float, float> >::iterator it=gc->pointsList.begin(); it != gc->pointsList.end(); ++it) {
        minScaleX = std::min(minScaleX, it->first);
        maxScaleX = std::max(maxScaleX, it->first);
        
        minScaleY = std::min(minScaleY, it->second);
        maxScaleY = std::max(maxScaleY, it->second);
    }
    
    if (gc->setFixedScaleMinMaxX){
        gc->maxX = std::max(maxScaleX, gc->maxX);
        gc->minX = std::min(minScaleX, gc->minX);
    }
    if (gc->setFixedScaleMinMaxY){
        gc->maxY = std::max(maxScaleY, gc->maxY);
        gc->minY = std::min(minScaleY, gc->minY);
    }
    if (gc->maxX != gc->minX) {
        minScaleX = gc->minX;
        maxScaleX = gc->maxX;
    }
    if (gc->maxY != gc->minY) {
        minScaleY = gc->minY;
        maxScaleY = gc->maxY;
    }

    int previousValue_x = -1, previousValue_y = -1;
    for (std::list<std::pair<float, float> >::iterator it=gc->pointsList.begin(); it != gc->pointsList.end(); ++it) {
        //~ normalisée = (originale - MIN) * (max - min) / (MAX - MIN) + min
        //~ [MIN,MAX] : interval d'origine
        //~ [min,max] : interval cible
        //~ originale : valeur dans l'interval d'origine
        //~ normalisée : valeur normalisée dans l'interval cible
        int value_x = (it->first - minScaleX) * (textureDesc.width - 1) / (maxScaleX - minScaleX);
        int value_y = (it->second - minScaleY) * (textureDesc.height - 1) / (maxScaleY - minScaleY);

        //~ putPoint(textureDesc, textureDesc.channels * (value_x + textureDesc.width * value_y), gc->lineWidth*textureDesc.width);
        putPoint(textureDesc, value_x, value_y, gc->lineWidth*textureDesc.width);
        if (previousValue_x != -1 && previousValue_y != -1) {
            drawLine(textureDesc, std::make_pair(previousValue_x, previousValue_y), std::make_pair(value_x, value_y), gc->lineWidth*textureDesc.width);
        }
        previousValue_x = value_x;
        previousValue_y = value_y;
    }
}

void GraphSystem::drawLine(ImageDesc &textureDesc, std::pair<int, int> firstPoint, std::pair<int, int> secondPoint, int lineWidth=1) {
    int dx, dy;
    if ( (dx = secondPoint.first - firstPoint.first) != 0) {
        if (dx > 0) {
            if ( (dy = secondPoint.second - firstPoint.second) != 0) {
                if (dy > 0) {
                    if (dx >= dy) {
                        int e = dx;
                        dx = e * 2;
                        dy = dy * 2;
                        for (int i=firstPoint.first; i != secondPoint.first; ++i) {
                        	putPoint(textureDesc, i, firstPoint.second, lineWidth);
                            
                            if ( (e -= dy) < 0) {
                                firstPoint.second += 1;
                                e += dx;
                            }
                        }
                    }
                    else {
                        int e = dy;
                        dy = e * 2;
                        dx = dx * 2;
                        for (int i=firstPoint.second; i != secondPoint.second; ++i) {
                        	putPoint(textureDesc, firstPoint.first, i, lineWidth);

                            if ( (e -= dx) < 0) {
                                firstPoint.first += 1;
                                e += dy;
                            }
                        }
                    }
                }
                else {
                    if (dx >= -dy) {
                        int e = dx;
                        dx = e * 2;
                        dy = dy * 2;
                        for (int i=firstPoint.first; i != secondPoint.first; ++i) {
							putPoint(textureDesc, i, firstPoint.second, lineWidth);
                            if ( (e+=dy) < 0) {
                                firstPoint.second -= 1;
                                e += dx;
                            }
                        }
                    }
                    else {
                        int e = dy;
                        dy = e * 2;
                        dx = dx * 2;
                        for (int i=firstPoint.second; i != secondPoint.second; --i) {
                        	putPoint(textureDesc, firstPoint.first, i, lineWidth);

                            if ( (e += dx) > 0) {
                                firstPoint.first += 1;
                                e += dy;
                            }
                        }
                    }
                }
            }
            else {
                for (int i=firstPoint.first; i != secondPoint.first; ++i) {
                    putPoint(textureDesc, i, firstPoint.second, lineWidth);
                }
            }
        }
        else {
            if ( (dy = secondPoint.second - firstPoint.second) != 0) {
                if (dy > 0) {
                    if (-dx >= dy) {
                        int e = dx;
                        dx = e * 2;
                        dy = dy * 2;
                        for (int i=firstPoint.first; i!= secondPoint.first; --i) {
                            putPoint(textureDesc, i,firstPoint.second, lineWidth);
                            if ( (e+=dy) >= 0) {
                                firstPoint.second += 1;
                                e += dx;
                            }
                        }
                    }
                    else {
                        int e = dy;
                        dx = dx * 2;
                        dy = e * 2;
                        for (int i=firstPoint.second; i!= secondPoint.second; ++i) {
                            putPoint(textureDesc, firstPoint.first, i, lineWidth);
                            if ( (e+=dx) >= 0) {
                                firstPoint.first -= 1;
                                e += dy;
                            }
                        }
                    }
                }
                else {
                    if (dx <= dy) {
                        int e = dx;
                        dx = e * 2;
                        dy = dy * 2;
                        for (int i=firstPoint.first; i!= secondPoint.first; --i) {                                            
                            putPoint(textureDesc, i, firstPoint.second, lineWidth);

                            if ( (e-=dy) >= 0) {
                                firstPoint.second -= 1;
                                e += dx;
                            }
                        }
                    }
                    else {
                        int e = dy;
                        dx = dx * 2;
                        dy = e * 2;
                        for (int i=firstPoint.second; i!= secondPoint.second; --i) {
                            putPoint(textureDesc, firstPoint.first, i, lineWidth);

                            if ( (e-=dx) >= 0) {
                                firstPoint.first -= 1;
                                e += dy;
                            }
                        }
                    }
                }
            }
            else {
                for (int i=firstPoint.first; i!= secondPoint.first; --i) {
                    putPoint(textureDesc, i, firstPoint.second, lineWidth);
                }
            }
        }
    }
    else {
        if ((dy = secondPoint.second - firstPoint.second) != 0) {
            if (dy < 0) {
                for (int i=firstPoint.second; i!= secondPoint.second; --i) {
                    putPoint(textureDesc, firstPoint.second, i, lineWidth);
                }
            }
            else {
                for (int i=firstPoint.second; i!= secondPoint.second; ++i) {
                    putPoint(textureDesc, firstPoint.second, i, lineWidth);
                }
            }
        }
    }
}

#ifdef INGAME_EDITORS
void GraphSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    GraphComponent* tc = Get(entity, false);
    if (!tc) return;
}
#endif

