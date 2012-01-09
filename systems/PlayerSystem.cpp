#include "PlayerSystem.h"
#include "PhysicsSystem.h"

INSTANCE_IMPL(PlayerSystem);
	
PlayerSystem::PlayerSystem() : ComponentSystem<PlayerComponent>("player_") { 
}

void PlayerSystem::DoUpdate(float dt) {
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Actor* a = (*it).first;			
		PlayerComponent* bc = (*it).second;
		PhysicsComponent* pc = PHYSICS(a);
		
		float weight = (bc->directionTarget == Vector2::Zero) ? 0.3 : 0.1;
		pc->speed = bc->directionTarget * weight + pc->speed * (1-weight);
	
	}
}