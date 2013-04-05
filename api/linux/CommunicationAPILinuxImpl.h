#pragma once

#include "../CommunicationAPI.h"
#include "base/Log.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
        inline bool isGameCenterLoggedIn() {
            LOGV(1, "You can't log to gamecenter on this platform.");
            return false;
        }

        inline bool openGameCenter() {
            LOGV(1, "You can't open gameCenter on this platform.");
            return false;
        }

        inline std::list<Achievement::Struct> getAllAchievements() {
            std::list<Achievement::Struct> list;
            LOGW("TODO");
            //storageAPI->blablabla
            return list;
        }

        inline std::list<Score::Struct> getScores(
            /*unsigned leaderboardID, Score::Visibility visibility, unsigned startRank, unsigned count*/
            unsigned, Score::Visibility, unsigned startRank, unsigned) {

            std::list<Score::Struct> list;
            list.push_front(Score::Struct("Player3", "10", startRank+2, Score::FRIEND));
            list.push_front(Score::Struct("Player2", "433", startRank+1, Score::ME));
            list.push_front(Score::Struct("Player1", "1000", startRank, Score::ALL));
            LOGW("TODO");
            //storageAPI->blablabla
            return list;
        }

        inline void submitScore(unsigned leaderboardID, Score::Struct score) {
            LOGV(1, "TODO" << leaderboardID << "+" << score);
            //storageAPI->blablabla
        }


		inline void giftizMissionDone() {
            LOGV(1, "Mission done!");
        }

		inline int giftizGetButtonState() {
            LOGV(1, "Will always randomly in [0;4[");
            return (int)TimeUtil::GetTime() % 4;
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
};
