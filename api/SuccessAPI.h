#pragma once

#include <glog/logging.h>

class SuccessAPI {

   public:
      virtual void successCompleted(const char* description, unsigned long successId) {
	 LOG(INFO) << "Success completed '" << description << "': " << successId;
      }

      virtual void openLeaderboard(int mode, int diff) {
        LOG(INFO) << "Openleaderboard mode=" << mode << ", diff=" << diff;
      }

      virtual void openDashboard() {}
};
