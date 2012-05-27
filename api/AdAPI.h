#pragma once

#include <base/Log.h>

class AdAPI {
    public:
        virtual void showAd() { LOGI("affichage de la pub !"); }
        virtual bool done() { return true; }
};
