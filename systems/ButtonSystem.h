#pragma once

#include "System.h"

#include "opengl/TextureLibrary.h"

class VibrateAPI;

struct ButtonComponent {
	ButtonComponent() : mouseOver(false), clicked(false), lastClick(0.f), enabled(false), textureActive(InvalidTextureRef), textureInactive(InvalidTextureRef), overSize(1.f) , vibration(0.035f), type(NORMAL) { }

    ////// READ ONLY variables
    // States of button
    bool mouseOver;
    bool clicked, touchStartOutside;
    // Last time when entity was clicked (anti-bounced variable)
    float lastClick;
	////// END OF READ ONLY variables

	////// READ/WRITE variables
    // if true, entity is clickable
    bool enabled;
	////// END OF READ/WRITE variables

	////// WRITE ONLY variables
	// inactive -> all the time
	// active -> texture when the button is touch
	TextureRef textureActive, textureInactive;
	// To make easier to click on the button
	float overSize;
	// Vibration on clicked
	float vibration;
	// type of button
	enum {
		NORMAL,
		LONGPUSH,
	} type;
	// Trigger
	float trigger;
	////// END OF WRITE ONLY variables

	// first touch
	float firstTouch;	
};

#define theButtonSystem ButtonSystem::GetInstance()
#define BUTTON(actor) theButtonSystem.Get(actor)
UPDATABLE_SYSTEM(Button)

public:
    VibrateAPI* vibrateAPI;
private:
	void UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const glm::vec2& touchPos);

#ifdef SAC_DEBUG
	std::map<Entity, TextureRef> oldTexture;
#endif
};
