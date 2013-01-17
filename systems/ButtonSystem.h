/*!
 * \file ButtonSystem.h
 * \brief System to handle buttons
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "System.h"

class VibrateAPI;

/*! \struct ButtonComponent
 *  \brief */
struct ButtonComponent {
	ButtonComponent() : enabled(false), mouseOver(false), clicked(false), overSize(1) , vibration(0.035), lastClick(0) { }

	bool enabled; //!< Button is enabled or not
	bool mouseOver; //!< Mouse is over the button
	bool clicked, touchStartOutside; //!< The button is clicked, 
    float overSize, vibration; //!< ?, Force of vibration on click
    float lastClick; //!< time since the last click (?)
};

#define theButtonSystem ButtonSystem::GetInstance()
#define BUTTON(actor) theButtonSystem.Get(actor)

/*! \class Button
 *  \brief */
UPDATABLE_SYSTEM(Button)

public:
    VibrateAPI* vibrateAPI; //!<
private:
    /*! \brief
     *  \param entity
     *  \param comp
     *  \param touching
     *  \param touchPos */
	void UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const Vector2& touchPos);
};

