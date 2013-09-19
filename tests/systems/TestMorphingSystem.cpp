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



#include <UnitTest++.h>
#include "systems/MorphingSystem.h"
#include <glm/glm.hpp>

TEST(SimpleMorphing)
{
    MorphingSystem::CreateInstance();
    Entity e = 1;
    theMorphingSystem.Add(e);
    MorphingComponent* m = MORPHING(e);

    float floatOutput;

    TypedMorphElement<float>* floatMorph = new TypedMorphElement<float>(&floatOutput, 10, -5);

    m->elements.push_back(floatMorph);
    m->active = true;
    m->timing = 1;

    theMorphingSystem.Update(0);
    CHECK_CLOSE(10, floatOutput, 0.0001);
    theMorphingSystem.Update(0.5);
    CHECK_CLOSE((10 - 5) * 0.5, floatOutput, 0.0001);
    theMorphingSystem.Update(0.5);
    CHECK_CLOSE(-5, floatOutput, 0.0001);
    //CHECK_EQUAL(0, m->elements.size());

    MorphingSystem::DestroyInstance();
}

TEST(DoubleMorphing)
{
    MorphingSystem::CreateInstance();
    Entity e = 1;
    theMorphingSystem.Add(e);
    MorphingComponent* m = MORPHING(e);

    float floatOutput;
    glm::vec2 vectorOut;

    TypedMorphElement<float>* floatMorph = new TypedMorphElement<float>(&floatOutput, 3, 6.7);
    TypedMorphElement<glm::vec2>* vecMorph = new TypedMorphElement<glm::vec2>(&vectorOut, glm::vec2(0., 0.), glm::vec2(5, -1));


    m->elements.push_back(floatMorph);
    m->elements.push_back(vecMorph);

    m->active = true;
    m->timing = 1;

    theMorphingSystem.Update(0);
    theMorphingSystem.Update(1);
    CHECK_CLOSE(6.7, floatOutput, 0.0001);
    CHECK_EQUAL(glm::vec2(5, -1).x, vectorOut.x);
    CHECK_EQUAL(glm::vec2(5, -1).y, vectorOut.y);

    //CHECK_EQUAL(0, m->elements.size());

    MorphingSystem::DestroyInstance();
}

