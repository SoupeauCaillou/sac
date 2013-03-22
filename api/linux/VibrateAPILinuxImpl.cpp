#include "VibrateAPILinuxImpl.h"
#include <glog/logging.h>

void VibrateAPILinuxImpl::vibrate(float d) {
    LOG(INFO) << "Vibrate : " << d;
}

