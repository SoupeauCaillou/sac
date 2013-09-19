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



#pragma once

#include "../CommunicationAPI.h"

#include "base/Log.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
		void giftizMissionDone() { LOGV(1, "Mission done!"); }
		int giftizGetButtonState() { LOGV(1, "Will always return 0"); return 0; }
		void giftizButtonClicked() { LOGV(1, "buttonClicked not handled on this platform."); }

		void shareFacebook() { LOGV(1, "facebook share"); }
		void shareTwitter() { LOGV(1, "twitter share"); }

		bool mustShowRateDialog() { LOGV(1, "always true"); return true; }
		void rateItNow() { LOGV(1, "not handled"); }
		void rateItLater() { LOGV(1, "not handled"); }
		void rateItNever() { LOGV(1, "not handled"); }
};
