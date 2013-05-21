#pragma once

#include "base/Log.h"

class SuccessAPI {

   public:
      virtual void successCompleted(const char* description, unsigned long successId) {
	       LOGI("Success completed '" << description << "': " << successId);
      }

      virtual void openLeaderboard(int mode, int diff) {
        LOGI("Openleaderboard mode=" << mode << ", diff=" << diff);
      }

      virtual void openDashboard() {}
};
