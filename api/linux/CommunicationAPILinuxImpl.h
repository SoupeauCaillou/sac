#pragma once

#include "../CommunicationAPI.h"
#ifdef WINDOWS
	#include <base/Log.h>
#else
	#include <glog/logging.h>
#endif


class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
		bool swarmInstalled() { VLOG(1) << "please enable swarm !"; return false; }
		void swarmRegistering() { VLOG(1) << "swarm registered"; }

		void giftizMissionDone() { }
		int  giftizGetButtonState() { return ((int)TimeUtil::GetTime()) % 4; }
		void giftizButtonClicked() { }

		void shareFacebook() { VLOG(1) << "facebook share"; }
		void shareTwitter() { VLOG(1) << "twitter share"; }

		bool mustShowRateDialog() { return true; }
		void rateItNow() {  }
		void rateItLater() {  }
		void rateItNever() {  }
};
