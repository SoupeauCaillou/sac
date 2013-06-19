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

        void init(JNIEnv* env);

        void init() {}

        bool isGameCenterLoggedIn();
        bool openGameCenter();

        inline std::list<Achievement> getAllAchievements() {
            std::list<Achievement> list;
            LOGV(1, "TODO");
            return list;
        }

        inline std::list<Score> getScores(
//            unsigned leaderboardID, Score::Visibility visibility, unsigned startRank, unsigned count) {
            unsigned, Score::Visibility, unsigned, unsigned) {

            std::list<Score> list;
            LOGV(1, "TODO");
            return list;
        }
        void submitScore(unsigned , Score) {}

		void giftizMissionDone();
		int  giftizGetButtonState();
		void giftizButtonClicked();

		void shareFacebook();
		void shareTwitter();

		bool mustShowRateDialog();
		void rateItNow();
		void rateItLater();
		void rateItNever();
    public:
        JNIEnv* env;
};
