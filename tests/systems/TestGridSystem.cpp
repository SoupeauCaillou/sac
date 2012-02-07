#include <UnitTest++.h>

#include "systems/GridSystem.h"

static void initGrid(char* type, int size) 
{
	theGridSystem.GridSize = size;
	theGridSystem.Clear();

	for(int i=0; i<size; i++) {
		for(int j=0; j<size; j++) {
			Entity e;
			theGridSystem.Add(e);
			GRID(e)->row = i;
			GRID(e)->column = j;
			GRID(e)->row = *type++;
		}
	}
}

TEST(RowCombination)
{
	char grid[] = {
		'A', 'B', 'B', 'B',
		'A', 'B', 'B', 'A',
		'A', 'A', 'B', 'A',
		'A', 'B', 'A', 'A',
	};
	initGrid(grid, 4);
	
	std::vector<Combination> result;
	CHECK(theGridSystem.LookForCombination() == 1);
	CHECK(result[0].type == 'B');
}

