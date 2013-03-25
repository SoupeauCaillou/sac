#pragma once

class AdAPI {
    public:
        virtual bool showAd() { 
            LOGI("affichage de la pub !")
            return true;
        }
        virtual bool done() { return true; }
};
