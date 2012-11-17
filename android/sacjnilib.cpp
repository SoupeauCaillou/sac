/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "sacjnilib.h"
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>

#include "base/Game.h"
#include "base/Log.h"
#include "base/Vector2.h"
#include "systems/RenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"
#include "base/TouchInputManager.h"
#include "base/EntityManager.h"
#include "base/MathUtil.h"

#include <png.h>
#include <algorithm>

#include <sys/time.h>

#ifndef _Included_net_damsy_soupeaucaillou_SacJNILib
#define _Included_net_damsy_soupeaucaillou_SacJNILib
#ifdef __cplusplus
extern "C" {
#endif

struct GameHolder;

struct AndroidNativeTouchState : public NativeTouchState{
	GameHolder* holder;
	AndroidNativeTouchState(GameHolder* h) {
		holder = h;
	}
    int maxTouchingCount() {
        return holder->input.size();
    }
	bool isTouching (int index, Vector2* windowCoords) const {
        // map stable order ?
        std::map<int, GameHolder::__input>::iterator it = holder->input.begin();
        for (int i=0; i<index && it!=holder->input.end(); ++it, i++) ;
         
        if (it == holder->input.end())
            return false;

		windowCoords->X = it->second.x;
		windowCoords->Y = it->second.y;

		return it->second.touching;
	}
};

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    createGame
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_createGame
  (JNIEnv *env, jclass, jint openglesVersion) {
  	LOGW("%s -->", __FUNCTION__);
  	//if (env == (JNIEnv*)0x2) {
	//  	plop();
  	//}
  	TimeUtil::init();
	GameHolder* hld = GameHolder::build();
	hld->initDone = false;

	theRenderingSystem.assetAPI = &hld->renderThreadJNICtx.asset;
	theTouchInputManager.setNativeTouchStatePtr(new AndroidNativeTouchState(hld));

    pthread_mutex_init(&hld->mutex, 0);
    pthread_cond_init(&hld->cond, 0);
    hld->renderingStarted = hld->drawQueueChanged = false;

	return (jlong)hld;
}

JNIEXPORT jlong JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_destroyGame
  (JNIEnv *env, jclass, jlong g) {
    GameHolder* hld = (GameHolder*) g;
    theMusicSystem.uninit();
    delete hld->game;
    delete hld;
}

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    init
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_initFromRenderThread
  (JNIEnv *env, jclass, jobject asset, jlong g, jint w, jint h) {
  LOGW("%s -->", __FUNCTION__);
	GameHolder* hld = (GameHolder*) g;
	hld->width = w;
	hld->height = h;

	hld->renderThreadJNICtx.init(env, asset);

	hld->game->sacInit(hld->width, hld->height);
	LOGW("%s <--", __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_uninitFromRenderThread
  (JNIEnv *env, jclass, jlong g) {
  LOGW("%s -->", __FUNCTION__);
	GameHolder* hld = (GameHolder*) g;
	hld->renderThreadJNICtx.uninit(env);
	LOGW("%s <--", __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_initFromGameThread
  (JNIEnv *env, jclass, jobject asset, jlong g, jbyteArray jstate) {
  	GameHolder* hld = (GameHolder*) g;
  	bool fullInit = true;

  	if (hld->gameThreadJNICtx->env && hld->gameThreadJNICtx->env != env && !jstate)
  		fullInit = false;
	hld->gameThreadJNICtx->init(env, asset);

	theMusicSystem.musicAPI = &hld->gameThreadJNICtx->musicAPI;
	theMusicSystem.assetAPI = &hld->gameThreadJNICtx->asset;
	theSoundSystem.soundAPI = &hld->gameThreadJNICtx->soundAPI;

	theSoundSystem.init();

	uint8_t* state = 0;
	int size = 0;
	if (jstate) {
		size = env->GetArrayLength(jstate);
		state = (uint8_t*)env->GetByteArrayElements(jstate, NULL);
		LOGW("Restoring saved state (size:%d)", size);
	} else if (hld->initDone) {
		return;
	} else {
		LOGW("No saved state: creating a new Game instance from scratch");
	}

	theMusicSystem.init();

	hld->firstCall = true;
	hld->dtAccumuled = 0;

	hld->game->init(state, size);
	hld->initDone = true;
 
    theRenderingSystem.Update(0);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_uninitFromGameThread
  (JNIEnv *env, jclass, jlong g) {
  LOGW("%s -->", __FUNCTION__);
	GameHolder* hld = (GameHolder*) g;
	hld->gameThreadJNICtx->uninit(env);
	LOGW("%s <--", __FUNCTION__);
}

float bbbefore = 0;
/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    step
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_step
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;

	if (!hld->game)
  		return;

    // begin updating when rendering has started
    pthread_mutex_lock(&hld->mutex);
    while (!hld->renderingStarted) {
        // LOGI("Renderer not ready. Waiting");
        pthread_cond_wait(&hld->cond, &hld->mutex);
    }

    const float t = TimeUtil::getTime();
    float delta = t - bbbefore;
    if (true|| delta > 0.012) {
        hld->renderingStarted = false;
        theRenderingSystem.currentWriteQueue = (theRenderingSystem.currentWriteQueue + 1) % 2;
        hld->drawQueueChanged = true;
        pthread_cond_signal(&hld->cond);
        pthread_mutex_unlock(&hld->mutex);
 // LOGW("%.4f != %.4f", delta, hld->renderingDt);
        // produce a single frame
        hld->game->tick(delta);
        bbbefore = t;
        theRenderingSystem.Update(0);
        delta = TimeUtil::getTime() - t;
    } else {
        pthread_mutex_unlock(&hld->mutex);
    }

    if (delta < 0.016) {
        // LOGW("Will sleep : %.4f", 0.016 - delta);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (0.016 - delta) * 1000000000LL;
        nanosleep(&ts, 0);
        delta = TimeUtil::getTime() - t;
    }    
// LOGW("<-- update - %.3f s", TimeUtil::getTime() - before);
}
#if 0
    pthread_mutex_init(&hld->mutex, 0);
    pthread_cond_init(&hld->cond, 0);
    hld->renderingStarted = false;

    const float DT = hld->game->targetDT;

  	if (hld->firstCall) {
		hld->time = TimeUtil::getTime();
		hld->firstCall = false;
	}

	float dt;
	do {
		dt = hld->dtAccumuled + TimeUtil::getTime() - hld->time;
		if (dt < DT) {
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = (DT - dt) * 1000000000LL;
			nanosleep(&ts, 0);
		}
	} while (dt < DT);

	hld->time = TimeUtil::getTime();

	const float accum = DT;// * 0.5;
	if (dt > 5 * DT) {
		LOGW("BIG DT: %.3f s", dt);
	}
LOGW("Update : %.3f", dt);
	while (dt >= DT){
		hld->game->tick(accum);
		dt -= accum;
	}
	hld->dtAccumuled = dt;
}
#endif

static float tttttt = 0, prev;
static int frameCount = 0;
static float minDt = 10, maxDt = 0;
float pauseTime;
// HACK: this one is called only from Activity::onResume
// Here we'll compute the time since pause. If < 5s -> autoresume the music
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_resetTimestep
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;

	if (!hld)
  		return;
  	hld->firstCall = true;
  	float d = tttttt = TimeUtil::getTime();
	frameCount = 0;
  	LOGW("resume time: %.3f, diff:%.3f, %d", d, d - pauseTime, theSoundSystem.mute);
  	if (d - pauseTime <= 5) {
	  	theMusicSystem.toggleMute(theSoundSystem.mute);
  	}
}

float renderingPrevTime = 0;
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_render
  (JNIEnv *env, jclass, jlong g) {
#define ENABLE_LOG 1
#define COUNT 500.0
    GameHolder* hld = (GameHolder*) g;

    if (!hld)
         return;
         

    pthread_mutex_lock(&hld->mutex);
    // LOGI("Rendering done");
    hld->renderingStarted = true;
    pthread_cond_signal(&hld->cond);
    while (!hld->drawQueueChanged) {
        LOGI("Renderer waiting :(");
        pthread_cond_wait(&hld->cond, &hld->mutex);
    }
    hld->drawQueueChanged = false;
    pthread_mutex_unlock(&hld->mutex);
    const float t = TimeUtil::getTime();
    hld->renderingDt = t - renderingPrevTime;
    renderingPrevTime = t;
    
    float before = TimeUtil::getTime();
// LOGW("render -->");
    theRenderingSystem.render();
    glFinish();
// LOGW("<-- render - %.3f s", TimeUtil::getTime() - before);

    {
        static int frameCount = 0;
        static float accum = 0, minDt=10000, maxDt=0;
        frameCount++;
        if (hld->renderingDt > maxDt)
            maxDt = hld->renderingDt;
        if (hld->renderingDt < minDt) {
            LOGW("Min dt: %.4f", hld->renderingDt);
            minDt = hld->renderingDt;
        }
        if (frameCount == 1000) {
            LOGW("FPS avg/min/max : %.2f / %.2f / %.2f",
                frameCount / (t - accum), 1.0 / maxDt, 1.0 / minDt);
            frameCount = 0;
            maxDt = 0;
            minDt = 10000;
            accum = t;
        }
    }
}

#if 0

	frameCount++;
    float t = TimeUtil::getTime();
    float dt = t - prev;
    if (dt < minDt)
        minDt = dt;
    if (dt > maxDt)
        maxDt = dt;
    prev = t;
	if (frameCount >= COUNT) {
        float fps = COUNT / (TimeUtil::getTime() - tttttt);
        if (fps > 0) {
            GameHolder* hld = (GameHolder*) g;
            float dt = 1/fps;
            float ratio = hld->game->targetDT / dt;
            __android_log_print(ANDROID_LOG_INFO, "sacC", "fps render: %.2f (ratio: %.2f) - min:%.3f max:%.3f", fps, ratio, minDt, maxDt);
            
            /*
            if (ratio < 0.90) {
                hld->game->targetDT += (dt - hld->game->targetDT) * 0.5;
                hld->game->targetDT = MathUtil::Min(1.0f/30.0f, hld->game->targetDT);
                LOGW("Reduce fps target: %.2f", 1.0 / hld->game->targetDT);
            } else if (ratio < 0.95) {
                hld->game->targetDT *= 0.99;
                hld->game->targetDT = MathUtil::Max(1.0f/60.0f, hld->game->targetDT);
                LOGW("Increase fps target: %.2f", 1.0 / hld->game->targetDT);
            } else {
                hld->game->targetDT *= 0.9;
                hld->game->targetDT = MathUtil::Max(1.0f/60.0f, hld->game->targetDT);
                LOGW("Increase fps target: %.2f", 1.0 / hld->game->targetDT);
            }
            */
        }
		tttttt = TimeUtil::getTime();
		frameCount = 0;
        minDt = 100;
        maxDt = 0;
	}
    theRenderingSystem.render();
}
#endif

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_pause
  (JNIEnv *env, jclass, jlong g) {
  	GameHolder* hld = (GameHolder*) g;
  	LOGW("%s -->", __FUNCTION__);
  	if (!hld->game)
  		return;

    // kill all music
    theMusicSystem.toggleMute(true);
	hld->game->togglePause(true);
	pauseTime = TimeUtil::getTime();
	LOGW("%s <-- %.3f", __FUNCTION__, pauseTime);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_back
  (JNIEnv *env, jclass, jlong g) {
     GameHolder* hld = (GameHolder*) g;
     LOGW("%s -->", __FUNCTION__);
     if (!hld->game)
         return;

    hld->game->backPressed();
    LOGW("%s <--", __FUNCTION__);
}


JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_invalidateTextures
  (JNIEnv *env, jclass, jobject asset, jlong g) {
     GameHolder* hld = (GameHolder*) g;
     LOGW("%s -->", __FUNCTION__);
     if (!hld->game || !RenderingSystem::GetInstancePointer())
         return;

    hld->renderThreadJNICtx.init(env, asset);

    // kill all music
    theRenderingSystem.invalidateAtlasTextures();
    LOGW("%s <--", __FUNCTION__);
}

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    handleInputEvent
 * Signature: (JIFF)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_handleInputEvent
  (JNIEnv *env, jclass, jlong g, jint evt, jfloat x, jfloat y, jint pointerIndex) {
	GameHolder* hld = (GameHolder*) g;

	if (g == 0)
		return;
    
    GameHolder::__input& input = hld->input[pointerIndex];

	/* ACTION_DOWN == 0 | ACTION_MOVE == 2 */
    if (evt == 0 || evt == 2) {
        input.touching = 1;
    	input.x = x;
   	    input.y = y;
    }
    /* ACTION_UP == 1 */
    else if (evt == 1) {
    	input.touching = 0;
   }
}

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    serialiazeState
 * Signature: (J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_serialiazeState
  (JNIEnv *env, jclass, jlong g) {
	LOGW("%s -->", __FUNCTION__);
	GameHolder* hld = (GameHolder*) g;
	uint8_t* state;
	int size = hld->game->saveState(&state);

	jbyteArray jb = 0;
	if (size) {
		jb = env->NewByteArray(size);
		env->SetByteArrayRegion(jb, 0, size, (jbyte*)state);
		LOGW("Serialized state size: %d", size);
	}

    // delete hld->game;
    // delete hld;

	LOGW("%s <--", __FUNCTION__);
	return jb;
}

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    restoreRenderingSystemState
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_initAndReloadTextures
  (JNIEnv *env, jclass, jlong g) {
  LOGW("%s -->", __FUNCTION__);
  GameHolder* hld = (GameHolder*) g;
  theRenderingSystem.init();
  theRenderingSystem.reloadTextures();
  theRenderingSystem.reloadEffects();
  
  LOGW("%s <--", __FUNCTION__);
}

#ifdef __cplusplus
}
#endif
#endif
