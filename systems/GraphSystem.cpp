#include "GraphSystem.h"

#include "base/EntityManager.h"

#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"

#define SIZE 256

void putPixel(ImageDesc &textureDesc, int pos, unsigned char value=0xFF) {
	if ( pos > -1 &&  pos < textureDesc.width* textureDesc.height* textureDesc.channels) {
        memset(textureDesc.datas + pos, value, 4);
    }
}

void putPoint(ImageDesc &textureDesc, int pos) {
    for (int i=-1; i<2; ++i) {
        for (int j=-1; j<2; ++j) {
                putPixel(textureDesc, pos + j*textureDesc.channels + i * textureDesc.width * textureDesc.channels);
                if (pos + textureDesc.channels * (j + i * textureDesc.width) + 3 > -1 && 
                	pos + textureDesc.channels * (j + i * textureDesc.width) + 3 < textureDesc.width * textureDesc.height * textureDesc.channels)
                	textureDesc.datas[pos + j*textureDesc.channels + i * textureDesc.width * textureDesc.channels + 3] = ((i==j || i==-j) && (i == -1 || i == 1)) ? 0x80 : 0xFF;
        }
    }    
}

INSTANCE_IMPL(GraphSystem);

GraphSystem::GraphSystem() : ComponentSystemImpl<GraphComponent>("Graph") {
    
}

void GraphSystem::DoUpdate(float dt) {
	for (std::map<TextureRef, ImageDesc>::iterator it=textureRef2Image.begin(); it != textureRef2Image.end(); ++it) {
		memset(it->second.datas, 0, it->second.width*it->second.height*it->second.channels);
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

        putPoint(textureDesc, textureDesc.channels * (value_x + textureDesc.width * value_y));
        if (previousValue_x != -1 && previousValue_y != -1) {
            drawLine(textureDesc, std::make_pair(previousValue_x, previousValue_y), std::make_pair(value_x, value_y));
        }
        previousValue_x = value_x;
        previousValue_y = value_y;
    }
}

void GraphSystem::drawLine(ImageDesc &textureDesc, std::pair<int, int> firstPoint, std::pair<int, int> secondPoint) {
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
                        	putPixel(textureDesc, textureDesc.channels * (i + textureDesc.width * firstPoint.second));
                            
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
                        	putPixel(textureDesc, textureDesc.channels * (firstPoint.first + textureDesc.width * i));

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
							putPixel(textureDesc, textureDesc.channels * (i + textureDesc.width * firstPoint.second));
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
                        	putPixel(textureDesc, textureDesc.channels * (firstPoint.first + textureDesc.width * i));

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
                    putPixel(textureDesc, textureDesc.channels * (i + textureDesc.width * firstPoint.second));
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
                            putPixel(textureDesc, textureDesc.channels * (i + textureDesc.width * firstPoint.second));
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
                            putPixel(textureDesc, textureDesc.channels * (firstPoint.first + textureDesc.width * i));
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
                            putPixel(textureDesc, textureDesc.channels * (i + textureDesc.width * firstPoint.second));

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
                            putPixel(textureDesc, textureDesc.channels * (firstPoint.first + textureDesc.width * i));

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
                    putPixel(textureDesc, textureDesc.channels * (i + textureDesc.width * firstPoint.second));
                }
            }
        }
    }
    else {
        if ((dy = secondPoint.second - firstPoint.second) != 0) {
            if (dy < 0) {
                for (int i=firstPoint.second; i!= secondPoint.second; --i) {
                    putPixel(textureDesc, textureDesc.channels * (firstPoint.second + textureDesc.width * i));
                }
            }
            else {
                for (int i=firstPoint.second; i!= secondPoint.second; ++i) {
                    putPixel(textureDesc, textureDesc.channels * (firstPoint.second + textureDesc.width * i));
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

