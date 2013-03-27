#include <UnitTest++.h>
#undef CHECK
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>
#include <base/EntityManager.h>

int main(int, char **) {
    glfwInit();
	EntityManager::CreateInstance();
	return UnitTest::RunAllTests();
}
