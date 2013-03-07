#pragma once

#include "System.h"
#include <list>
#include <utility>
#include <map>
#include "util/ImageLoader.h"

struct GraphComponent {
    
    std::list<std::pair<float, float> > pointsList;

    float minScaleX, maxScaleX;
    float minScaleY, maxScaleY;
    uint listSize;
};

#define theGraphSystem GraphSystem::GetInstance()
#define GRAPH_SYSTEM(e) theGraphSystem.Get(e)

UPDATABLE_SYSTEM(Graph)

    void drawTexture(unsigned char *textureTab, GraphComponent *points);
    void drawLine(unsigned char *textureTab);

    std::map<Entity, ImageDesc> entity2Image;
};
