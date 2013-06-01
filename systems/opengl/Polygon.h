#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Shape {
    enum Enum {
        Square = 0,
        Triangle,
        Hexagon,
        Count
    };
}

struct Polygon {
    static Polygon create(Shape::Enum e);

    std::vector<glm::vec2> vertices;
    std::vector<unsigned short> indices;
};
