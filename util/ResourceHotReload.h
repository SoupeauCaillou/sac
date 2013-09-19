/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include <map>
#include <string>

class ResourceHotReload {
    public:
    	virtual ~ResourceHotReload() {}

        void updateReload();

        virtual void reload(const std::string& assetName) = 0;

        virtual std::string asset2File(const std::string& assetName) const = 0;

        void registerNewAsset(const std::string & assetName, const std::string & override = "");

#if SAC_LINUX && SAC_DESKTOP
    private:
        //for inotify
        struct InotifyDatas {
            int wd;
            int inotifyFd;
            std::string _filename;
            std::string _assetname;

            InotifyDatas(const std::string & file, const std::string & asset);
        };

        std::map<std::string, InotifyDatas> filenames;
#endif
};
