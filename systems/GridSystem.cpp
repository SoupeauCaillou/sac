#include <iostream>
#include <vector>
#include "GridSystem.h"

INSTANCE_IMPL(GridSystem);
	
GridSystem::GridSystem() : ComponentSystem<GridComponent>("Grid") { 
	GridSize=8;
}

Entity GridSystem::GetOnPos(int i, int j) {
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		GridComponent* bc = (*it).second;
		if (bc->row == i && bc->column == j)
			return a;
	}
	return 0;
}
	
void GridSystem::ResetTest() {
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		GridComponent* bc = (*it).second;
		bc->checkedH = false;
		bc->checkedV = false;
	}
}

std::vector<Combinais> GridSystem::LookForCombinaison(int nbmin) { 
	std::vector<Combinais> combinaisons;
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		GridComponent* gc = (*it).second;
		int i=gc->row;
		int j=gc->column;

		/*Check on column*/
		if (!gc->checkedV) {

			int longueurCombi=0;
			Vector2 origine = Vector2::Zero;
			
			/*Looking for twins on the bottom of the point*/
			int k=j;
			while (k>-1){
				Entity next = GetOnPos(i,k);
		
				if (GRID(next)->type != gc->type){
					origine=Vector2(i,k+1);
					k=-2;
				} else {
					/*Useless to check them later : we already did it now*/
					GRID(next)->checkedV = true;
					longueurCombi++;
					k--;
				}
			}
			/*If the first was good*/
			if (k==-1)
				origine=Vector2(i,0);
				
			
			/* Then on the top*/
			k = j+1;
			while (k<GridSize){
				Entity next = GetOnPos(i,k);

				if (GRID(next)->type != gc->type){
					k=GridSize;
				} else {
					GRID(next)->checkedV = true;
					longueurCombi++;
					k++;
				}
			}
			
				
			/*If there is at least 1 more cell
			 * We add it to the solutions*/
			 
			if (longueurCombi>=nbmin){

				Combinais tmp;
				tmp.origine = origine;
				tmp.size = longueurCombi;
				tmp.type = gc->type;
				combinaisons.push_back(tmp);
			}
			
			gc->checkedV = true;
		} 
		
		/*Check on row*/
		if (!gc->checkedH) {

			int longueurCombi=0;
			Vector2 origine = Vector2::Zero;
			
			/*Looking for twins on the left of the point*/
			int k=i;
			while (k>-1){
				Entity next = GetOnPos(k,j);
				
				if (GRID(next)->type != gc->type) {
					origine=Vector2(k+1,j);
					k=-2;
				} else {
					/*Useless to check them later : we already did it now*/
					GRID(next)->checkedH = true;
					longueurCombi++;
					k--;
				}
			}
			/*If the first was good*/
			if (k==-1)
				origine=Vector2(0,j);
				
			
			/* Then on the right*/
			k = i+1;
			while (k<GridSize){
				Entity next = GetOnPos(k,j);

				if (GRID(next)->type != gc->type) {
					k=(GridSize+1);
				} else {
					GRID(next)->checkedH = true;
					longueurCombi++;
					k++;
				}
			}			

	
			/*If there is at least 1 more cell
			 * We add it to the solutions
			 * longueurCombi < 0 <-> Horizontale */
			 
			if (longueurCombi>=nbmin){
				Combinais tmp;
				tmp.origine = origine;
				tmp.size = -longueurCombi;
				tmp.type = gc->type;
				combinaisons.push_back(tmp);
			}
			
			gc->checkedH = true;
		} 
		
		
	}
	
	return combinaisons;
}

void GridSystem::DoUpdate(float dt) {
	std::vector<Combinais> combinaisons;
	//combinaisons = LookForCombinaison();
	
	if (combinaisons.size()>0){
		for ( std::vector<Combinais>::reverse_iterator it = combinaisons.rbegin(); it != combinaisons.rend(); ++it )
		{
			std::cout << "(" <<it->origine.X << ", "<< it->origine.Y << ") :" << it->size << std::endl;
			
			/*Traitement des combinaions*/

			/*for (int i=0; i<it->size;i++){
				Entity cour;
				if (it->size<0)
					cour = GetOnPos(it->origine.X+i,it->origine.Y);
				else
					cour = GetOnPos(it->origine.X,it->origine.Y+i);

			//	RENDERING(cour)->texture = theRenderingSystem.loadTextureFile("1.png");
			}
			//it.pop_back()*/
		}
	}	
	
	
}


