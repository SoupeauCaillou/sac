/*
	This file is part of RecursiveRunner.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	RecursiveRunner is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	RecursiveRunner is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "api/StorageAPI.h"

//TODO: tout mettre en const
class StorageAPILinuxImpl : public StorageAPI {
	public:
		void init(const std::string & databaseName);

		void submitScore(Score inScore);
		std::vector<Score> getScores(float& outAverage);

		bool isFirstGame();
		void incrementGameCount();

		bool isMuted();
		void setMuted(bool b);

#if SAC_EMSCRIPTEN
        bool muted;
        std::vector<Score> scores;
#else
        void checkInTable(const std::string & option,
            const std::string & valueIfExist, const std::string & valueIf404);

        bool request(const std::string & statement, void* res, int
            (*callbackP)(void*,int,char**,char**));
	private:
        std::string _dbPath;
#endif
};
