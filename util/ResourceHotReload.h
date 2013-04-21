#pragma once

#include <map>
#include <string>

class ResourceHotReload {
    public:
    	virtual ~ResourceHotReload() {}

        void updateReload();

        virtual void reload(const std::string& file) = 0;

        virtual std::string assetPrefix() const { return ""; }

        virtual std::string assetSuffix() const { return ""; }

        void registerNewAsset(const std::string & name);

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
