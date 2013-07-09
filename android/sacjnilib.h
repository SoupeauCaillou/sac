#pragma once
#include <jni.h>

#include <map>

struct Game;

class GameHolder {
	public:
	static GameHolder* build();

	Game* game;
	int width, height;
    JNIEnv* gameEnv, *renderEnv;
	// GameThreadJNIEnvCtx* gameThreadJNICtx;
	// RenderThreadJNIEnvCtx renderThreadJNICtx;

	struct __input {
        __input() : touching(0), moving(0) {}
		 int touching, moving;
		 float x, y;
	};
    std::map<int, __input> input;
	float dtAccumuled, time;

    float renderingDt;

	bool initDone;

	private:
	GameHolder() {}
};
