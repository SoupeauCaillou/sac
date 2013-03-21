#include "VibrateAPILinuxImpl.h"
#ifdef WINDOWS
#include <base/Log.h>
#else
#include <glog/logging.h>
#endif

void VibrateAPILinuxImpl::vibrate(float d) {
    LOG(INFO) << "Vibrate : " << d;
}

