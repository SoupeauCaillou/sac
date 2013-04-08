#pragma once

#include <string>
#include <list>
#include <ostream>
#include <sstream>

class CommunicationAPI {
	public:
        struct Achievement {
            unsigned _id;
            bool _isUnlocked;
            std::string _name;
            std::string _description;

            friend std::ostream & operator<<(std::ostream & o, const Achievement & a) {
                return o << a._id << ". " << a._name << ": " << a._description << ". It's " << (a._isUnlocked ? "unlocked" : "locked");
            }
        };

        struct Score {
            enum Visibility {
                ALL,
                FRIEND,
                ME
            };

            std::string _name;
            std::string _score;
            unsigned _rank;
            Visibility _visibility;

            Score() {}
            Score(const std::string& name, const float & score, unsigned rank, Visibility visibility)
                : _name(name), _rank(rank), _visibility(visibility) {
                    std::stringstream ss;
                    ss << score;
                    _score = ss.str();
            }
            Score(const std::string& name, const std::string& score, unsigned rank, Visibility visibility)
                : _name(name), _score(score), _rank(rank), _visibility(visibility) {
            }

            friend std::ostream & operator<<(std::ostream & o, const Score & s) {
                o << s._rank << ". (";

                if (s._visibility == Visibility::ALL)
                    o << "ALL";
                else if (s._visibility == Visibility::FRIEND)
                    o << "FRIEND";
                else
                    o << "ME";

                return o << ") '" << s._name << "' did " << s._score;
            }
        };

        class ScoreHandler {
            public:
                //retrieve all achievements
                virtual std::list<Achievement> getAllAchievements() = 0;
                //retrieve $count scores from a leaderboard, starting at rank $startRank
                virtual std::list<Score> getScores(unsigned leaderboardID,
                    Score::Visibility visibility, unsigned startRank, unsigned count) = 0;
                //submit a score on a specific leaderboard
                virtual void submitScore(unsigned leaderboardID, Score score) = 0;
        };

        //only for pc yet
        virtual void init(ScoreHandler* scoreHandler) = 0;

		//is the player logged into the gamecenter?
		virtual bool isGameCenterLoggedIn() = 0;
        //open the gamecenter. Return true in success
        virtual bool openGameCenter() = 0;

        //retrieve all achievements
        virtual std::list<Achievement> getAllAchievements() = 0;
        //retrieve $count scores from a leaderboard, starting at rank $startRank
        virtual std::list<Score> getScores(unsigned leaderboardID,
            Score::Visibility visibility, unsigned startRank, unsigned count) = 0;
        //submit a score on a specific leaderboard
        virtual void submitScore(unsigned leaderboardID, Score score) = 0;

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
