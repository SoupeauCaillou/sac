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
        ResourceHotReload();

        virtual ~ResourceHotReload() {}

        void updateReload();

#if SAC_DEBUG
        virtual void reload(const char* assetName) = 0;
#endif

        void registerNewAsset(const std::string & assetName, const std::string & override = "");

        virtual const char* asset2FilePrefix() const = 0;
        virtual const char* asset2FileSuffix() const = 0;

    protected:
        void asset2File(const char* assetName, char* out, int maxSize) const;

#if SAC_LINUX && SAC_DESKTOP
    private:
        //for inotify
        struct InotifyDatas {
            int wd;
            std::string _filename;
            std::string _assetname;

            InotifyDatas(int fd, const std::string & file, const std::string & asset);
        };

        int inotifyFd;

        std::map<std::string, InotifyDatas> filenames;
#endif
};
