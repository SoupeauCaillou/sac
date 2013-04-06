#pragma once

#include "../CommunicationAPI.h"

#include "api/linux/StorageAPILinuxImpl.h"

#include "base/Log.h"

class CommunicationAPILinuxImpl : public CommunicationAPI {
	public:
        inline void init(ScoreHandler* scoreHandler)  {
            _scoreHandler = scoreHandler;
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
            return _scoreHandler->getAllAchievements();
        }

        inline std::list<Score> getScores(
            unsigned leaderboardID, Score::Visibility visibility, unsigned startRank, unsigned count) {

            return _scoreHandler->getScores(leaderboardID, visibility, startRank, count);
/*


            std::list<Score> list;
            list.push_front(Score("Player3", "10", startRank+2, Score::FRIEND));
            list.push_front(Score("Player2", "433", startRank+1, Score::ME));
            list.push_front(Score("Player1", "1000", startRank, Score::ALL));
            LOGW("TODO");
            //storageAPI->blablabla
            return list;
*/
        }

        inline void submitScore(unsigned leaderboardID, Score score) {
            return _scoreHandler->submitScore(leaderboardID, score);
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

        //will handle the scores/achievements
        ScoreHandler* _scoreHandler;
};
