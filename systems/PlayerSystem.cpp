#include "PlayerSystem.h"

INSTANCE_IMPL(PlayerSystem);
	
PlayerSystem::PlayerSystem() : ComponentSystemImpl<PlayerComponent>("player_") { 
}

int PlayerSystem::GetScore() {
	std::vector<Entity> vec = thePlayerSystem.RetrieveAllActorWithComponent();
	if (vec.size()==1) return PLAYER(vec[0])->score;
	else return 0;
}

float PlayerSystem::GetTime() {
	std::vector<Entity> vec = thePlayerSystem.RetrieveAllActorWithComponent();
	if (vec.size()==1) return PLAYER(vec[0])->time;
	else return 0;
}

void PlayerSystem::Reset() {
	std::vector<Entity> vec = thePlayerSystem.RetrieveAllActorWithComponent();
	if (vec.size()==1)
	PLAYER(vec[0])->time = 0;
	PLAYER(vec[0])->score = 0;
	
	PLAYER(vec[0])->isReadyToStart = false;
	PLAYER(vec[0])->level = 1;
	PLAYER(vec[0])->multiplier = 1;
}

void PlayerSystem::ScoreCalc(int nb) {
	std::vector<Entity> vec = thePlayerSystem.RetrieveAllActorWithComponent();
	
	if (vec.size()==1)		PLAYER(vec[0])->score += PLAYER(vec[0])->level*nb*PLAYER(vec[0])->multiplier;
	else 	std::cout << "Il y n'y a pas 1 seul personne dans le playersystem\n";
}


void PlayerSystem::DoUpdate(float dt) {
	std::vector<Entity> vec = thePlayerSystem.RetrieveAllActorWithComponent();
	if (vec.size()==1)		PLAYER(vec[0])->time += dt;
	else	std::cout << "Il y n'y a pas 1 seul personne dans le playersystem\n";
}
