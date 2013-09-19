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



#pragma once

#include "System.h"
#include <list>
#include <utility>
#include <map>
#include <string>
#include "opengl/TextureLibrary.h"
#include "util/ImageLoader.h"
#include "base/Color.h"

struct GraphComponent {

    GraphComponent():lineWidth(0), maxY(0), maxX(0), minY(0), minX(0), 
    setFixedScaleMinMaxX(false), setFixedScaleMinMaxY(false), reloadTexture(true),
    lineColor(Color(1, 1, 1)) {}

    std::list<std::pair<float, float> > pointsList;
	
	std::string textureName;
	
	float lineWidth; // between ]0:1] (percent) if 0 -> 1 pixel
	
    float maxY, maxX, minY, minX;

    bool setFixedScaleMinMaxX;
    bool setFixedScaleMinMaxY;
    
    bool reloadTexture;
    
    Color lineColor;
};

#define theGraphSystem GraphSystem::GetInstance()
#define GRAPH(e) theGraphSystem.Get(e)

UPDATABLE_SYSTEM(Graph)

    void drawTexture(ImageDesc &textureDesc, GraphComponent *points);
    void drawLine(ImageDesc &textureDesc, std::pair<int, int> firstPoint, std::pair<int, int> secondPoint, int lineWidth, Color color);

    std::map<TextureRef, ImageDesc> textureRef2Image;
};
