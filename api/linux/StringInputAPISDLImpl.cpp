/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "StringInputAPISDLImpl.h"

#include "base/Log.h"

#include <SDL.h>

void StringInputAPISDLImpl::askUserInput(const std::string& initialText, const int imaxSize) {
    LOGI("Please enter something...");
    currentText = initialText;
    textIsReady = false;
    maxSize = imaxSize;
    SDL_StartTextInput();
}

void StringInputAPISDLImpl::cancelUserInput() {
    textIsReady = true;
    currentText.clear();
    SDL_StopTextInput();
}

bool StringInputAPISDLImpl::done(std::string & final) {
    final = currentText;
    if (textIsReady && currentText.size() > 0) {
        SDL_StopTextInput();
        return true;
    }
    return false;
}

int StringInputAPISDLImpl::eventSDL(const void* inEvent) {
    if (textIsReady) {
        return 0;
    }

    auto event = (SDL_Event*)inEvent;

    LOGT("Handle [Enter] press!??");
    if (event->type == SDL_TEXTEDITING) {
        currentText = event->edit.text;
        LOGI("current text: " << currentText);
        return 1;
    }

    return 0;
}

void StringInputAPISDLImpl::setNamesList(const std::vector<std::string> & LOG_USAGE_ONLY(names)) {
#if SAC_ENABLE_LOG
    for (auto & n : names) {
        LOGT("setNamesList, adding name '" << n << "'");
    }
#endif
}
