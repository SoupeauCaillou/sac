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

#include "../AssetAPI.h"
#include "JNIWrapper.h"

#include "base/Log.h"

namespace jni_asset_api {
    enum Enum {
        LoadAsset,
        ListAssetContent,
        GetWritableAppDatasPath,
    };
}

class AssetAPIAndroidImpl : public AssetAPI, public JNIWrapper<jni_asset_api::Enum> {
	public:
		AssetAPIAndroidImpl();

        std::list<std::string> listAssetContent(const std::string& extension, const std::string& subfolder);

    	FileBuffer loadAsset(const std::string& asset);

        const std::string & getWritableAppDatasPath();

        FileBuffer loadFile(const std::string&);

        std::list<std::string> listContent(const std::string&, const std::string&,
            const std::string&);

        void createDirectory(const std::string& ) { LOGT(""); }
        bool doesExistFileOrDirectory(const std::string& ) { LOGT(""); return false; }
        void removeFileOrDirectory(const std::string& ) { LOGT(""); }


        void synchronize() {}
};
