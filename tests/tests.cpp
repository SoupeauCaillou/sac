#include <UnitTest++.h>
#include "base/EntityManager.h"
#include "base/Log.h"

int main(int argc, char **) {
    __log_enabled = (argc > 1);
	EntityManager::CreateInstance();
	return UnitTest::RunAllTests();
}
