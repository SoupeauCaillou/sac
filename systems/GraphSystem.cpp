/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "GraphSystem.h"
#include "systems/RenderingSystem.h"

#include <algorithm>
#include <cstdint>
#include <cmath>

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
    END_FOR_EACH()
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
        if (gc->minX < minScaleX / 2 || gc->maxX > maxScaleX * 2)
            LOGW_EVERY_N(120, "Check your X-scale on graph '" <<
                gc->textureName << "' : min/maxX = " << gc->minX << '/' << gc->maxX << " minScale/maxScaleX = " <<
                minScaleX << '/' << maxScaleX);
        minScaleX = gc->minX;
        maxScaleX = gc->maxX;
    }
    if (gc->maxY != gc->minY) {
        if (gc->minY < minScaleY / 2 || gc->maxY > maxScaleY * 2)
            LOGW_EVERY_N(120, "Check your Y-scale on graph '" <<
                gc->textureName << "' : min/maxY = " << gc->minY << '/' << gc->maxY << " minScale/maxScaleY = " <<
                minScaleY << '/' << maxScaleY);
        minScaleY = gc->minY;
        maxScaleY = gc->maxY;
    }

    minScaleY -= 0.1 * std::abs(minScaleY);
    maxScaleY += 0.1 * std::abs(minScaleY);

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
