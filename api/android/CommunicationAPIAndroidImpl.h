#pragma once

#include "../CommunicationAPI.h"
#include "base/Log.h"
#include <jni.h>

class CommunicationAPIAndroidImpl : public CommunicationAPI {
	public:
		CommunicationAPIAndroidImpl();
		~CommunicationAPIAndroidImpl();
		void init(JNIEnv* env);
		void uninit();


        bool isGameCenterLoggedIn();
        bool openGameCenter();

        inline std::list<Achievement::Struct> getAllAchievements() {
            std::list<Achievement::Struct> list;
            LOGV(1, "TODO");
            return list;
        }

        inline std::list<Score::Struct> getScores(unsigned leaderboardID,
            Score::Visibility visibility, unsigned startRank, unsigned count) {
            std::list<Score::Struct> list;
            LOGV(1, "TODO");
            return list;
        }
        void submitScore(unsigned , Score::Struct ) {}

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
