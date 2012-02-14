#include "PlayerSystem.h"

INSTANCE_IMPL(PlayerSystem);
	
PlayerSystem::PlayerSystem() : ComponentSystemImpl<PlayerComponent>("player_") { 
}

void PlayerSystem::DoUpdate(float dt) {
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		PlayerComponent* bc = (*it).second;
	}
}
