#pragma once

#include <string>
#include <list>

namespace Achievement {
    struct Struct {
        unsigned _id;
        bool _isUnlocked;
        std::string _name;
        std::string _description;
    };
}

namespace Score {
    enum Visibility {
        ALL,
        FRIEND,
        ME
    };

    struct Struct {
        std::string _name;
        std::string _score;
        unsigned _rank;
        Visibility _visibility;
    };
}

class CommunicationAPI {
	public:
		//is the player logged into the gamecenter?
		virtual bool isGameCenterLoggedIn() = 0;
        //open the gamecenter. Return true in success
        virtual bool openGameCenter() = 0;

        //retrieve all achievements
        virtual std::list<Achievement::Struct> getAllAchievements() = 0;
        //retrieve $count scores from a leaderboard, starting at rank $startRank
        virtual std::list<Score::Struct> getScores(unsigned leaderboardID,
            Score::Visibility visibility, unsigned startRank, unsigned count) = 0;
        virtual void submitScore(unsigned leaderboardID, Score::Struct score) = 0;

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
