#pragma once

#include "../CommunicationAPI.h"
#include <jni.h>

class CommunicationAPIAndroidImpl : public CommunicationAPI {
	public:
		CommunicationAPIAndroidImpl();
		~CommunicationAPIAndroidImpl();
		void init(JNIEnv* env);
		void uninit();

		bool swarmInstalled();
		void swarmRegistering(int mode, int difficulty);

		void giftizMissionDone();
		int  giftizGetButtonState();
		void giftizButtonClicked();

		void shareFacebook();
		void shareTwitter();

		bool mustShowRateDialog();
		void rateItNow();
		void rateItLater();
		void rateItNever();

	private:
		class CommunicationAPIAndroidImplDatas;
		CommunicationAPIAndroidImplDatas* datas;

	public :
		JNIEnv* env;
};
