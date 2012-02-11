#include <UnitTest++.h>

#include "systems/GridSystem.h"

static void initGrid(char* type, int size) 
{
	theGridSystem.Clear();
	theGridSystem.GridSize = size;

	for(int j=size-1; j>=0; j--) {
		for(int i=0; i<size; i++) {
			Entity e =  i * size + j + 1;
			theGridSystem.Add(e);
			GRID(e)->row = i;
			GRID(e)->column = j;
			GRID(e)->type = *type++;
			//std::cout << "("<<i<<";"<<j<<") : "<<	GRID(e)->type << "\t";
		}
		//std::cout << std::endl;
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
	
	std::vector<Combinais> combinaisons;
	combinaisons = theGridSystem.LookForCombinaison(3);
	CHECK(combinaisons.size()==3);
	theGridSystem.ResetTest();
	combinaisons = theGridSystem.LookForCombinaison(2);
	CHECK(combinaisons.size()==3);
	theGridSystem.ResetTest();
	combinaisons = theGridSystem.LookForCombinaison(1);
	CHECK(combinaisons.size()==4);
}

