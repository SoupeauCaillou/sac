#include <UnitTest++.h>
#undef CHECK
#define GLEW_STATIC
#include <SDL/SDL.h>
#include <base/EntityManager.h>

int main(int argc, char **) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	EntityManager::CreateInstance();
    if (argc == 1)
        logLevel = LogVerbosity::FATAL;
    else
        logLevel = LogVerbosity::VERBOSE1;
	return UnitTest::RunAllTests();
}
