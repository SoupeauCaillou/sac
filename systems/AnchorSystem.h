#pragma once

#include <glm/glm.hpp>

#include "System.h"


struct AnchorComponent {
	AnchorComponent(): parent(0), position(0.0f), anchor(0.0f), rotation(0.0f), z(0) {}

	// parent
	Entity parent;
	// position in parent's coordinates
	glm::vec2 position;
	// anchor point in our own coords
	glm::vec2 anchor;
	// rotation around anchor
	float rotation;
	// z offset from parent
	float z;
};

#define theAnchorSystem AnchorSystem::GetInstance()
#define ANCHOR(e) theAnchorSystem.Get(e)

UPDATABLE_SYSTEM(Anchor)

public:
    static glm::vec2 adjustPositionWithAnchor(const glm::vec2& position, const glm::vec2& anchor);

#if SAC_DEBUG
    void Delete(Entity e) override;
#endif
};
