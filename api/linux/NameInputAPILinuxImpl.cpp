/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "NameInputAPILinuxImpl.h"
#include "base/Log.h"
#include <iostream>
#include "../../systems/TextRenderingSystem.h"
#include "../../systems/ButtonSystem.h"

void NameInputAPILinuxImpl::show() {
    __log_enabled = false;    
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
    __log_enabled = true;
    TEXT_RENDERING(title)->hide = TEXT_RENDERING(nameEdit)->hide = true;
    RENDERING(background)->hide = true;
    // BUTTON(button)->enabled = false;
}
