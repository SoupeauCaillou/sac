#pragma once

#include "System.h"
#include "stdafx.h"

struct HierarchyComponent {
	HierarchyComponent() {
		parent = 0;
		localPosition = Vector2::Zero;
		localRotation = 0;
		zOffset = 0;
	}
	Actor* parent;
	Vector2 localPosition;
	float localRotation;
	int zOffset;
};

#define theHierarchySystem HierarchySystem::GetInstance()
#define HIERARCHY(e) theHierarchySystem.Get(e)

UPDATABLE_SYSTEM(Hierarchy)
};
