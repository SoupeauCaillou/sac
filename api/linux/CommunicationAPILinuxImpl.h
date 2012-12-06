#pragma once

#include "../CommunicationAPI.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
		bool swarmInstalled() { LOGI("please enable swarm !"); return false; }
		void swarmRegistering(int mode __attribute__((unused)), int difficulty __attribute__((unused))) { LOGI("swarm activated"); }

		void giftizMissionDone() { }
		int  giftizGetButtonState() { return 0; };
		void giftizButtonClicked() { };

		void shareFacebook() { LOGI("facebook share"); }
		void shareTwitter() { LOGI("twitter share"); }

		bool mustShowRateDialog() { return false; }
		void rateItNow() {  }
		void rateItLater() {  }
		void rateItNever() {  }
};
