#pragma once

#include "../CommunicationAPI.h"

#include "base/Log.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
        inline void init()  {
        }

        inline bool isGameCenterLoggedIn() {
            LOGV(1, "You can't log to gamecenter on this platform.");
            return false;
        }

        inline bool openGameCenter() {
            LOGV(1, "You can't open gameCenter on this platform.");
            return false;
        }

        inline std::list<Achievement> getAllAchievements() {
            LOGV(1, "Nothing done on pc");
            return std::list<Achievement>();
        }

        inline std::list<Score> getScores(
            unsigned, Score::Visibility, unsigned, unsigned) {
            LOGV(1, "Nothing done on pc");
            return std::list<Score>();
        }

        inline void submitScore(unsigned, Score) {
            LOGV(1, "Nothing done on pc");
        }

		inline void giftizMissionDone() {
            LOGV(1, "Mission done!");
        }

		inline int giftizGetButtonState() {
            LOGV(1, "Will always return 0");
            return 0;
        }

		inline void giftizButtonClicked() {
            LOGV(1, "buttonClicked not handled on this platform.");
        }

		void shareFacebook() { LOGV(1, "facebook share") }
		void shareTwitter() { LOGV(1, "twitter share") }

		bool mustShowRateDialog() { LOGV(1, "always true"); return true; }
		void rateItNow() { LOGV(1, "not handled"); }
		void rateItLater() { LOGV(1, "not handled"); }
		void rateItNever() { LOGV(1, "not handled"); }

        StorageAPI* _storageAPI;
};
