#pragma once

#include "../CommunicationAPI.h"
#include "JNIWrapper.h"

namespace jni_comm_api {
    enum Enum {
        IsGameCenterLoggedIn,
        OpenGameCenter,
        // GetAllAchievements,
        // GetScores,
        // SubmitScore,
        GiftizMissionDone,
        GiftizGetButtonState,
        GiftizButtonClicked,
        ShareFacebook,
        ShareTwitter,
        MustShowRateDialog,
        RateItNow,
        RateItLater,
        RateItNever,
    };
}
class CommunicationAPIAndroidImpl : public CommunicationAPI, public JNIWrapper<jni_comm_api::Enum> {
	public:
		CommunicationAPIAndroidImpl();

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
};
