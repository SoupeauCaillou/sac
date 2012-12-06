#pragma once

#include "../VibrateAPI.h"

class VibrateAPILinuxImpl : public VibrateAPI {
    public:
        void vibrate(float duration);
};
