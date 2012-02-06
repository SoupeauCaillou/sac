#pragma once

#include "System.h"
#include "base/MathUtil.h"
typedef struct Vector3 {
	Vector2 origine;
	int size;
} Vector3;

struct GridComponent {
	GridComponent() {
		row = 0 ;
		column = 0;
		type = 0;
		checkedV = false;
		checkedH = false;
	}
	int row, column, type;
	bool checkedV, checkedH;

};

#define theGridSystem GridSystem::GetInstance()
#define GRID(e) theGridSystem.Get(e)

UPDATABLE_SYSTEM(Grid)

public:
Entity GetOnPos(int i, int j);


};
