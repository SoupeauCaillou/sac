#pragma once

#include "System.h"
#include "RenderingSystem.h"


	struct ButtonComponent {
		ButtonComponent() : enabled(true), mouseOver(false) { }
		
		bool enabled;
		bool mouseOver;
		bool clicked, touchStartOutside;
	};
	
	#define theButtonSystem ButtonSystem::GetInstance()
	#define BUTTON(actor) theButtonSystem.Get(actor)
	UPDATABLE_SYSTEM(Button)
			
	private:
		void UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const Vector2& touchPos);

	public:
		static bool inside(const Vector2& testPoint, const Vector2& rectPos, const Vector2& rectSize);
	};

