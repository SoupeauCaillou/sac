#pragma once

#include "../CommunicationAPI.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
		bool swarmInstalled() { LOGI("please enable swarm !"); return false; }
		void swarmRegistering() { LOGI("swarm registered"); }

		void giftizMissionDone() { }
		int  giftizGetButtonState() { return ((int)TimeUtil::getTime()) % 4; }
		void giftizButtonClicked() { }

		void shareFacebook() { LOGI("facebook share"); }
		void shareTwitter() { LOGI("twitter share"); }

		bool mustShowRateDialog() { return true; }
		void rateItNow() {  }
		void rateItLater() {  }
		void rateItNever() {  }
};
