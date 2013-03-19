#pragma once

class AdAPI {
    public:
        virtual bool showAd() { 
            LOG(INFO) << "affichage de la pub !";
            return true;
        }
        virtual bool done() { return true; }
};
