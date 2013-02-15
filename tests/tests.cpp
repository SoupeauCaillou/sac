#include <UnitTest++.h>
#undef CHECK
#include "base/EntityManager.h"

#include <glog/logging.h>
#include <gflags/gflags.h>

int main(int argc, char ** argv) {
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    google::ParseCommandLineFlags(&argc, &argv, true);

	EntityManager::CreateInstance();
	return UnitTest::RunAllTests();
}
