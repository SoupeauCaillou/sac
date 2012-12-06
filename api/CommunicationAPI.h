#pragma once

class CommunicationAPI {
	public:
		virtual bool swarmInstalled() = 0;
		virtual void swarmRegistering(int mode, int difficulty) = 0;

		virtual void giftizMissionDone() = 0;
		virtual int  giftizGetButtonState() = 0;
		virtual void giftizButtonClicked() = 0;

		virtual void shareFacebook() = 0;
		virtual void shareTwitter() = 0;

		virtual bool mustShowRateDialog() = 0;
		virtual void rateItNow() = 0;
		virtual void rateItLater() = 0;
		virtual void rateItNever() = 0;
};

