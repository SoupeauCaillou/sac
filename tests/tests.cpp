#include <UnitTest++.h>
#include "base/EntityManager.h"

int main() {
	EntityManager::CreateInstance();
	return UnitTest::RunAllTests();
}
