#pragma once

#include "../CommunicationAPI.h"
#include "base/Log.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
		bool swarmInstalled() { LOGV(1, "please enable swarm !") return false; }
		void swarmRegistering() { LOGV(1, "swarm registered") }

		void giftizMissionDone() { }
		int  giftizGetButtonState() { return ((int)TimeUtil::GetTime()) % 4; }
		void giftizButtonClicked() { }

		void shareFacebook() { LOGV(1, "facebook share") }
		void shareTwitter() { LOGV(1, "twitter share") }

		bool mustShowRateDialog() { return true; }
		void rateItNow() {  }
		void rateItLater() {  }
		void rateItNever() {  }
};
