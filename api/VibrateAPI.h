#pragma once

#include <base/Log.h>

class VibrateAPI {
    public:
        virtual void vibrate(float duration) = 0;
};
