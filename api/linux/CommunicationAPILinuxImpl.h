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
