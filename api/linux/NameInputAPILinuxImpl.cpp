#include "NameInputAPILinuxImpl.h"
#include <iostream>
#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

void NameInputAPILinuxImpl::show() {
    TEXT_RENDERING(title)->hide = TEXT_RENDERING(nameEdit)->hide = false;
    RENDERING(background)->hide = false;
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
    TEXT_RENDERING(title)->hide = TEXT_RENDERING(nameEdit)->hide = true;
    RENDERING(background)->hide = true;
    // BUTTON(button)->enabled = false;
}


