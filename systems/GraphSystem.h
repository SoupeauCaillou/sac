#pragma once

#include "System.h"
#include <list>
#include <utility>
#include <map>
#include "util/ImageLoader.h"

struct GraphComponent {

    GraphComponent():maxY(0), maxX(0), minY(0), minX(0), setFixedScaleMinMaxX(false), setFixedScaleMinMaxY(false) {}

    std::list<std::pair<float, float> > pointsList;

    float maxY, maxX, minY, minX;

    bool setFixedScaleMinMaxX;
    bool setFixedScaleMinMaxY;

};

#define theGraphSystem GraphSystem::GetInstance()
#define GRAPH_SYSTEM(e) theGraphSystem.Get(e)

UPDATABLE_SYSTEM(Graph)

    void drawTexture(unsigned char *textureTab, GraphComponent *points);
    void drawLine(unsigned char *textureTab, std::pair<int, int> firstPoint, std::pair<int, int> secondPoint);

    std::map<Entity, ImageDesc> entity2Image;
};
