#include "NameInputAPILinuxImpl.h"
#include <iostream>
#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

void NameInputAPILinuxImpl::show() {
    TEXT_RENDERING(title)->show = TEXT_RENDERING(nameEdit)->show = true;
    RENDERING(background)->show = true;
    // BUTTON(button)->enabled = true;
    textIsReady = false;
}

bool NameInputAPILinuxImpl::done(std::string& name) {
    if (textIsReady && TEXT_RENDERING(nameEdit)->text.length() > 0) {
	    name = TEXT_RENDERING(nameEdit)->text;
	    return true;
    }
    textIsReady = false;
    return false;
}

void NameInputAPILinuxImpl::hide() {
    TEXT_RENDERING(title)->show = TEXT_RENDERING(nameEdit)->show = false;
    RENDERING(background)->show = false;
    // BUTTON(button)->enabled = false;
}


