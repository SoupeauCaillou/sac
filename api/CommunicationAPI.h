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

#include <string>
#include <list>
#include <ostream>
#include <sstream>

class CommunicationAPI {
	public:
		//confirm to giftiz the mission is done
		virtual void giftizMissionDone() = 0;
		//retrieve giftiz button state (invisible=0/naked=1/badge=2/warning=3/else=-1)
		virtual int  giftizGetButtonState() = 0;
		//tell to giftiz that he was clicked
		virtual void giftizButtonClicked() = 0;

		//share on facebook (not implemanted yet)
		virtual void shareFacebook() = 0;
		//share on twitter (not implemanted yet)
		virtual void shareTwitter() = 0;

		//do we need to show the app rate dialog
		virtual bool mustShowRateDialog() = 0;
		//if we clicked on "rateItNow"
		virtual void rateItNow() = 0;
		//if we clicked on "rateItLater"
		virtual void rateItLater() = 0;
		//if we clicked on "rateItNever"
		virtual void rateItNever() = 0;
};
