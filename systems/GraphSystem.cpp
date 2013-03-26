#include "GraphSystem.h"

#include "base/EntityManager.h"

#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"

#include <algorithm>
#include <cstdint>

#ifndef uint
	#define uint unsigned long
#endif

#define SIZE 256

static void putPixel(ImageDesc &textureDesc, int pos_x, int pos_y, Color color) {
	static unsigned char colorTab[4];
	for (uint i=0; i<sizeof(colorTab); ++i) {
		colorTab[i] = color.rgba[i] * 255;
	}
	if ( pos_x > -1 &&  pos_x < textureDesc.width && pos_y > -1 &&  pos_y < textureDesc.height) {
        memcpy(textureDesc.datas + (pos_x + (textureDesc.width * (textureDesc.height-1 - pos_y))) * textureDesc.channels, colorTab, sizeof(colorTab));
    }
}

static void putPoint(ImageDesc &textureDesc, int pos_x, int pos_y, int lineWidth, Color color) {
	if (lineWidth == 0)
		lineWidth = 2;
    for (int i=-lineWidth / 2; i < lineWidth / 2; ++i) {
        for (int j=-lineWidth / 2; j < lineWidth / 2; ++j) {
			putPixel(textureDesc, pos_x + i, pos_y + j, color);
		}
    }
}

INSTANCE_IMPL(GraphSystem);

GraphSystem::GraphSystem() : ComponentSystemImpl<GraphComponent>("Graph") {

}

static void clear(ImageDesc& desc) {
    memset(desc.datas, 0x40, desc.width * desc.height * desc.channels);
}

void GraphSystem::DoUpdate(float) {
	for (std::map<TextureRef, ImageDesc>::iterator it=textureRef2Image.begin(); it != textureRef2Image.end(); ++it) {
		clear(it->second);
	}

    FOR_EACH_COMPONENT(Graph, gc)
        TextureRef texture = theRenderingSystem.textureLibrary.load(gc->textureName);

        std::map<TextureRef, ImageDesc>::iterator jt;
        if ( (jt = textureRef2Image.find(texture)) != textureRef2Image.end()) {
            if (gc->reloadTexture) drawTexture(jt->second, gc);
        }
        else {
            ImageDesc desc;
            desc.width = desc.height = SIZE;
            desc.channels = 4;
            desc.mipmap = 0;
            desc.type = ImageDesc::RAW;
            desc.datas = new char[desc.width * desc.height * desc.channels];
            clear(desc);
            drawTexture(desc, gc);

            theRenderingSystem.textureLibrary.registerDataSource(texture, desc);

            textureRef2Image.insert(std::make_pair(texture, desc));
        }
        if(gc->reloadTexture) {
        	theRenderingSystem.textureLibrary.reload(gc->textureName);
        	gc->reloadTexture = false;
        }
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

    minScaleY *= (minScaleY<0 ? 1.1f : 0.9f);
    maxScaleY *= (maxScaleY<0 ? 0.9f : 1.1f);

	if (gc->maxX != gc->minX) {
		int value_xmin = (gc->minX - minScaleX) * (textureDesc.height - 1) / (maxScaleX - minScaleX);
		int value_xmax = (gc->maxX - minScaleX) * (textureDesc.height - 1) / (maxScaleX - minScaleX);

		drawLine(textureDesc, std::make_pair(value_xmax, 0), std::make_pair(value_xmax, 255), gc->lineWidth*textureDesc.width, gc->lineColor);
		drawLine(textureDesc, std::make_pair(value_xmin, 0), std::make_pair(value_xmin, 255), gc->lineWidth*textureDesc.width, gc->lineColor);
	}
	if (gc->maxY != gc->minY) {
		int value_ymin = (gc->minY - minScaleY) * (textureDesc.height - 1) / (maxScaleY - minScaleY);
		int value_ymax = (gc->maxY - minScaleY) * (textureDesc.height - 1) / (maxScaleY - minScaleY);

		drawLine(textureDesc, std::make_pair(0, value_ymax), std::make_pair(255, value_ymax), gc->lineWidth*textureDesc.width, gc->lineColor);
		drawLine(textureDesc, std::make_pair(0, value_ymin), std::make_pair(255, value_ymin), gc->lineWidth*textureDesc.width, gc->lineColor);
	}

    int previousValue_x = -1, previousValue_y = -1;
    for (std::list<std::pair<float, float> >::iterator it=gc->pointsList.begin(); it != gc->pointsList.end(); ++it) {
        int value_x = (it->first - minScaleX) * (textureDesc.width - 1) / (maxScaleX - minScaleX);
        int value_y = (it->second - minScaleY) * (textureDesc.height - 1) / (maxScaleY - minScaleY);

        if (previousValue_x != -1 && previousValue_y != -1) {
            drawLine(textureDesc, std::make_pair(previousValue_x, previousValue_y), std::make_pair(value_x, value_y), gc->lineWidth*textureDesc.width, gc->lineColor);
        }
        previousValue_x = value_x;
        previousValue_y = value_y;
    }
}

void GraphSystem::drawLine(ImageDesc &textureDesc, std::pair<int, int> firstPoint, std::pair<int, int> secondPoint, int lineWidth, Color color) {
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
                        	putPoint(textureDesc, i, firstPoint.second, lineWidth, color);

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
                        	putPoint(textureDesc, firstPoint.first, i, lineWidth, color);

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
							putPoint(textureDesc, i, firstPoint.second, lineWidth, color);
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
                        	putPoint(textureDesc, firstPoint.first, i, lineWidth, color);

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
                    putPoint(textureDesc, i, firstPoint.second, lineWidth, color);
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
                        for (int i=firstPoint.first; i != secondPoint.first; --i) {
                            putPoint(textureDesc, i, firstPoint.second, lineWidth, color);
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
                        for (int i=firstPoint.second; i != secondPoint.second; ++i) {
                            putPoint(textureDesc, firstPoint.first, i, lineWidth, color);
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
                        for (int i=firstPoint.first; i != secondPoint.first; --i) {
                            putPoint(textureDesc, i, firstPoint.second, lineWidth, color);

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
                        for (int i=firstPoint.second; i != secondPoint.second; --i) {
                            putPoint(textureDesc, firstPoint.first, i, lineWidth, color);

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
                    putPoint(textureDesc, i, firstPoint.second, lineWidth, color);
                }
            }
        }
    }
    else {
        if ((dy = secondPoint.second - firstPoint.second) != 0) {
            if (dy < 0) {
                for (int i=firstPoint.second; i!= secondPoint.second; --i) {
                    putPoint(textureDesc, firstPoint.first, i, lineWidth, color);
                }
            }
            else {
                for (int i=firstPoint.second; i!= secondPoint.second; ++i) {
                    putPoint(textureDesc, firstPoint.first, i, lineWidth, color);
                }
            }
        }
    }
}

#ifdef SAC_INGAME_EDITORS
void GraphSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    GraphComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRO(bar, "textureName", TW_TYPE_STDSTRING, &tc->textureName, "group=Graph");
    TwAddVarRW(bar, "lineWidth", TW_TYPE_FLOAT, &tc->lineWidth, "group=Graph step=0,001");
    TwAddVarRW(bar, "maxY", TW_TYPE_FLOAT, &tc->maxY, "group=Graph");
    TwAddVarRW(bar, "maxX", TW_TYPE_FLOAT, &tc->maxX, "group=Graph");
    TwAddVarRW(bar, "minY", TW_TYPE_FLOAT, &tc->minY, "group=Graph");
    TwAddVarRW(bar, "minX", TW_TYPE_FLOAT, &tc->minX, "group=Graph");
    TwAddVarRW(bar, "setFixedScaleMinMaxX", TW_TYPE_BOOLCPP, &tc->setFixedScaleMinMaxX, "group=Graph");
    TwAddVarRW(bar, "setFixedScaleMinMaxY", TW_TYPE_BOOLCPP, &tc->setFixedScaleMinMaxY, "group=Graph");
    TwAddVarRW(bar, "lineColor", TW_TYPE_COLOR4F, &tc->lineColor, "group=Graph");
}
#endif
