#include "GraphSystem.h"

#include "base/EntityManager.h"

#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"

#define SIZE 256
#define CHANNEL 4

void putPoint(char r, char b, char g, unsigned char *textureTab, int pos) {
    for (int i=-1; i<2; ++i) {
        for (int j=-1; j<2; ++j) {
            if ( (pos + j*CHANNEL + i * SIZE * CHANNEL) > -1 &&  (pos + j*CHANNEL + i * SIZE * CHANNEL) < SIZE*SIZE*CHANNEL) {
                textureTab[pos + j*CHANNEL + i * SIZE * CHANNEL + 0] = r;
                textureTab[pos + j*CHANNEL + i * SIZE * CHANNEL + 1] = b;
                textureTab[pos + j*CHANNEL + i * SIZE * CHANNEL + 2] = g;
                textureTab[pos + j*CHANNEL + i * SIZE * CHANNEL + 3] = ((i==j || i==-j) && (i == -1 || i == 1)) ? 0xa0 : 0xFF;
            }
        }
    }
    
}

INSTANCE_IMPL(GraphSystem);

GraphSystem::GraphSystem() : ComponentSystemImpl<GraphComponent>("Graph") {
    
}

void GraphSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Graph, entity, gc)
        std::map<Entity, ImageDesc>::iterator jt;
        if ( (jt = entity2Image.find(entity)) != entity2Image.end()) {
            drawTexture(jt->second.udatas, gc);
        }
        else {
            LOG_IF(FATAL, RENDERING(entity)->texture != InvalidTextureRef) << "Entity mustn't have texture yet";

            RENDERING(entity)->texture = theRenderingSystem.loadTextureFile(theEntityManager.entityName(entity));
            ImageDesc desc;
            desc.width = desc.height = SIZE;
            desc.channels = 4;
            desc.mipmap = 0;
            desc.type = ImageDesc::RAW;
            desc.datas = new char[desc.width * desc.height * desc.channels];

            drawTexture(desc.udatas, gc);

            theRenderingSystem.textureLibrary.registerDataSource(RENDERING(entity)->texture, desc);

            entity2Image.insert(std::make_pair(entity, desc));
        }
        theRenderingSystem.textureLibrary.reload(theEntityManager.entityName(entity));
    }
}

void GraphSystem::drawTexture(unsigned char *textureTab, GraphComponent *gc) {
    memset(textureTab, 0x80, SIZE * SIZE * CHANNEL);
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
        int value_x = (it->first - minScaleX) * (SIZE-1) / (maxScaleX-minScaleX);
        int value_y = (it->second - minScaleY) * (SIZE-1) / (maxScaleY-minScaleY);

        if (value_x >= SIZE)
            value_x = SIZE - 1;
        if (value_x < 0)
            value_x = 0;
        if (value_y >= SIZE) 
            value_y = SIZE-1;
        if (value_y < 0)
            value_y = 0;
        putPoint(0xFF, 0, 0, textureTab, CHANNEL * (value_x + SIZE * value_y));
        if (previousValue_x != -1 && previousValue_y != -1) {
            drawLine(textureTab, std::make_pair(previousValue_x, previousValue_y), std::make_pair(value_x, value_y));
        }
        previousValue_x = value_x;
        previousValue_y = value_y;
    }

    
}

void GraphSystem::drawLine(unsigned char *textureTab, std::pair<int, int> firstPoint, std::pair<int, int> secondPoint) {
    //~ std::pair<int, int> firstPoint, secondPoint;
    //~ for (int x=0; x<SIZE; ++x) {
        //~ for (int y=0; y<SIZE; ++y) {
            //~ if (textureTab[CHANNEL * (x + SIZE * y)] == 0xFF) {
                //~ 
                //~ if (firstPoint.first == -1){
                    //~ firstPoint.first = x;
                    //~ firstPoint.second = y;
                //~ }
                //~ else {
                    //~ secondPoint.first = x;
                    //~ secondPoint.second = y;
                //~ }
                //~ 
                //~ if (secondPoint.first > -1) {
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
                            memset(textureTab + CHANNEL * (i + SIZE * firstPoint.second), 0xFF, 4);
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
                            memset(textureTab + CHANNEL * (firstPoint.first + SIZE * i), 0xFF, 4);

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
                            memset(textureTab + CHANNEL * (i + SIZE * firstPoint.second), 0xFF, 4);

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
                            memset(textureTab + CHANNEL * (firstPoint.first + SIZE * i), 0xFF, 4);

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
                    memset(textureTab + CHANNEL * (i + SIZE * firstPoint.second), 0xFF, 4);
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
                            memset(textureTab + CHANNEL * (i + SIZE * firstPoint.second), 0xFF, 4);
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
                            memset(textureTab + CHANNEL * (firstPoint.first + SIZE * i), 0xFF, 4);
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
                            memset(textureTab + CHANNEL * (i + SIZE * firstPoint.second), 0xFF, 4);

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
                            memset(textureTab + CHANNEL * (firstPoint.first + SIZE * i), 0xFF, 4);

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
                    memset(textureTab + CHANNEL * (i + SIZE * firstPoint.second), 0xFF, 4);
                }
            }
        }
    }
    else {
        if ((dy = secondPoint.second - firstPoint.second) != 0) {
            if (dy < 0) {
                for (int i=firstPoint.second; i!= secondPoint.second; --i) {
                    memset(textureTab + CHANNEL * (firstPoint.second + SIZE * i), 0xFF, 4);
                }
            }
            else {
                for (int i=firstPoint.second; i!= secondPoint.second; ++i) {
                    memset(textureTab + CHANNEL * (firstPoint.second + SIZE * i), 0xFF, 4);
                }
            }
        }
    }
                    //~ firstPoint.first = secondPoint.first;
                    //~ firstPoint.second = secondPoint.second;
                    //~ secondPoint.first = secondPoint.second = -1;
                //~ }
            //~ }
        //~ }
    //~ }
}

#ifdef INGAME_EDITORS
void GraphSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    GraphComponent* tc = Get(entity, false);
    if (!tc) return;

}
#endif

