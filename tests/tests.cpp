#include <UnitTest++.h>
#undef CHECK
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>
#include <base/EntityManager.h>

int main(int argc, char **) {
    glfwInit();
	EntityManager::CreateInstance();
    if (argc == 1)
        logLevel = LogVerbosity::FATAL;
    else
        logLevel = LogVerbosity::VERBOSE1;
	return UnitTest::RunAllTests();
}
