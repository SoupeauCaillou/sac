#pragma once

class CommunicationAPI {
	public:
		//return true if player is alreay logged on swarm
		virtual bool swarmInstalled() = 0;
		//launch swarm
		virtual void swarmRegistering() = 0;

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

