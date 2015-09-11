/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "sacjnilib.h"
#include <errno.h>

#include <GLES/gl.h>

#include <android/log.h>

#include "base/Game.h"
#include "base/Log.h"
#include <glm/glm.hpp>
#include "systems/RenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"
#include "base/TouchInputManager.h"
#include "base/EntityManager.h"
#include "base/Common.h"
#include "AndroidNativeTouchState.h"

#include "api/android/AdAPIAndroidImpl.h"
#include "api/android/AssetAPIAndroidImpl.h"
#include "api/android/CommunicationAPIAndroidImpl.h"
#include "api/android/ExitAPIAndroidImpl.h"
#include "api/android/GameCenterAPIAndroidImpl.h"
#include "api/android/InAppPurchaseAPIAndroidImpl.h"
#include "api/android/LocalizeAPIAndroidImpl.h"
#include "api/android/MusicAPIAndroidImpl.h"
#include "api/android/OpenURLAPIAndroidImpl.h"
#include "api/android/SoundAPIAndroidImpl.h"
#include "api/android/StringInputAPIAndroidImpl.h"
#include "api/android/VibrateAPIAndroidImpl.h"
#include "api/android/WWWAPIAndroidImpl.h"
#include "api/default/SqliteStorageAPIImpl.h"

#include "systems/opengl/OpenGLTextureCreator.h"

#include <algorithm>

#include "base/TimeUtil.h"
#include <sys/time.h>

/* Defined in EntryPoint.cpp */
extern Game* buildGameInstance();

static GameHolder* myGameHolder = 0;

#ifndef _Included_net_damsy_soupeaucaillou_SacJNILib
#define _Included_net_damsy_soupeaucaillou_SacJNILib
#ifdef __cplusplus
extern "C" {
#endif


/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    createGame
 * Signature: ()J
 */
JNIEXPORT jboolean JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_createGame
  (JNIEnv *, jclass) {
#if SAC_DEBUG && SAC_ENABLE_LOG
    logLevel = LogVerbosity::VERBOSE1;
#endif

    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);

    if (!myGameHolder) {
        TimeUtil::Init();
        LOGI("Create new native game instance");
        myGameHolder = new GameHolder();
        myGameHolder->game = buildGameInstance();
        myGameHolder->initDone = false;

        Game* game = myGameHolder->game;

        GameContext* gCtx = new GameContext, *rCtx = new GameContext;
        if (game->wantsAPI(ContextAPI::Ad))
            gCtx->adAPI = new AdAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::Asset) || true) {
            gCtx->assetAPI = new AssetAPIAndroidImpl();
            rCtx->assetAPI = new AssetAPIAndroidImpl();
        }
        if (game->wantsAPI(ContextAPI::Communication))
            gCtx->communicationAPI = new CommunicationAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::Exit))
            gCtx->exitAPI = new ExitAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::GameCenter))
            gCtx->gameCenterAPI = new GameCenterAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::InAppPurchase))
            gCtx->inAppPurchaseAPI = new InAppPurchaseAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::Localize))
            gCtx->localizeAPI = new LocalizeAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::Music))
            gCtx->musicAPI = new MusicAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::Storage))
            gCtx->storageAPI = new SqliteStorageAPIImpl();
        if (game->wantsAPI(ContextAPI::OpenURL))
            gCtx->openURLAPI = new OpenURLAPIAndroidImpl();
        //if (game->wantsAPI(ContextAPI::Network))
            gCtx->networkAPI = 0;
        if (game->wantsAPI(ContextAPI::Sound))
            gCtx->soundAPI = new SoundAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::StringInput))
           gCtx->stringInputAPI = new StringInputAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::Vibrate))
            gCtx->vibrateAPI = new VibrateAPIAndroidImpl();
        if (game->wantsAPI(ContextAPI::WWW))
            gCtx->wwwAPI = new WWWAPIAndroidImpl();

        theRenderingSystem.assetAPI = rCtx->assetAPI;

        game->setGameContexts(gCtx, rCtx);

        theTouchInputManager.setNativeTouchStatePtr(new AndroidNativeTouchState(myGameHolder));
        LOGW("<--" <<  __FUNCTION__ << '/' << myGameHolder);
        return true;
    } else {
        LOGW("<--" <<  __FUNCTION__);
        return false;
    }
}

#define INIT_1(var, ctx, type) \
    if (myGameHolder->game-> ctx##ThreadContext-> var) { \
        LOGI("JNI init: " << #type); \
        (static_cast< type *>(myGameHolder->game-> ctx##ThreadContext-> var))->init(env);\
    }

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    init
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_initFromRenderThread
  (JNIEnv *env, jclass, jint dpi, jint w, jint h) {
    myGameHolder->width = w;
    myGameHolder->height = h;
    myGameHolder->renderEnv = env;

    INIT_1(assetAPI, render, AssetAPIAndroidImpl)

    if (dpi > 2 || dpi < 0) {
        LOGW("Invalid DPI value. Using hdpi by default");
        dpi = 2;
    }
    OpenGLTextureCreator::dpi = (DPI::Enum)dpi;

    sac::setResolution(w, h);
    myGameHolder->game->sacInit();
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_resolutionChanged
  (JNIEnv *, jclass, jint w, jint h) {
    /* HUM */
    sac::setResolution(w, h);
    if (myGameHolder->game) {
        myGameHolder->game->changeResolution(w, h);
    }
}


static void initGameJni(JNIEnv *env) {
    myGameHolder->gameEnv = env;
    INIT_1(adAPI, game, AdAPIAndroidImpl)
    INIT_1(assetAPI, game, AssetAPIAndroidImpl)
    INIT_1(communicationAPI, game, CommunicationAPIAndroidImpl)
    INIT_1(exitAPI, game, ExitAPIAndroidImpl)
    INIT_1(gameCenterAPI, game, GameCenterAPIAndroidImpl)
    INIT_1(inAppPurchaseAPI, game, InAppPurchaseAPIAndroidImpl)
    INIT_1(localizeAPI, game, LocalizeAPIAndroidImpl)
    INIT_1(musicAPI, game, MusicAPIAndroidImpl)
    INIT_1(openURLAPI, game, OpenURLAPIAndroidImpl)
    INIT_1(soundAPI, game, SoundAPIAndroidImpl)
    INIT_1(stringInputAPI, game, StringInputAPIAndroidImpl)
    INIT_1(vibrateAPI, game, VibrateAPIAndroidImpl)
    INIT_1(wwwAPI, game, WWWAPIAndroidImpl)

}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_initFromGameThread
  (JNIEnv *env, jclass, jbyteArray jstate) {
    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    // init JNI env
    initGameJni(env);

    theMusicSystem.musicAPI = myGameHolder->game->gameThreadContext->musicAPI;
    theMusicSystem.assetAPI = myGameHolder->game->gameThreadContext->assetAPI;
    theSoundSystem.soundAPI = myGameHolder->game->gameThreadContext->soundAPI;

    // really needed ?
    theSoundSystem.init();

    // restore from state if any ?
    uint8_t* state = 0;
    int size = 0;
    if (jstate) {
        /*
        size = env->GetArrayLength(jstate);
        state = (uint8_t*)env->GetByteArrayElements(jstate, NULL);
        LOGT("Missing ReleaseCall !");
        LOGW("Restoring saved state (size:" << size << ")");
        */
    }
    // we don't need to re-init the game
    else if (myGameHolder->initDone) {
        myGameHolder->game->quickInit();
        LOGW("<-- (early) " <<  __FUNCTION__);
        return;
    }
    // full init
    else {
        LOGW("No saved state: creating a new Game instance from scratch");
    }

    theMusicSystem.init();

    myGameHolder->dtAccumuled = 0;

    myGameHolder->game->init(state, size);
    myGameHolder->initDone = true;

    myGameHolder->game->resetTime();

    LOGW("<--" <<  __FUNCTION__);
}

#if 0
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_uninitFromGameThread
  (JNIEnv *env, jclass, jlong g) {
    LOGW("-->" <<  __FUNCTION__)
    myGameHolder->gameThreadJNICtx->uninit(env);
    LOGW("<--" <<  __FUNCTION__)
}
#endif

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    step
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_step
  (JNIEnv * LOG_USAGE_ONLY(env), jclass) {
    if (!myGameHolder->game)
        return;
    LOGE_IF(env != myGameHolder->gameEnv, "Incoherent JNIEnv " << env << " != " << myGameHolder->gameEnv);
    myGameHolder->game->step();
}

static float pauseTime;
// HACK: this one is called only from Activity::onResume
// Here we'll compute the time since pause. If < 5s -> autoresume the music
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_resetTimestep
  (JNIEnv *env, jclass) {

    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    if (!myGameHolder)
        return;

    if (env != myGameHolder->gameEnv) {
        LOGI("Re-init JNI layer");
        initGameJni(env);
    }
    float d = TimeUtil::GetTime();
    myGameHolder->game->resetTime();
    LOGW("resume time: " << d << ", diff:" << (d - pauseTime) << ", " << theSoundSystem.mute);
    if ((d - pauseTime) <= 5) {
        theMusicSystem.toggleMute(theSoundSystem.mute);
    }
    LOGW("<--" <<  __FUNCTION__);
}

float renderingPrevTime = 0;
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_render
  (JNIEnv *LOG_USAGE_ONLY(env), jclass) {
    if (!myGameHolder)
         return;
    if (!RenderingSystem::GetInstancePointer())
        return;

    LOGE_IF(env != myGameHolder->renderEnv, "Incoherent JNIEnv " << env << " != " << myGameHolder->renderEnv);
    myGameHolder->game->render();
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_pause
  (JNIEnv *LOG_USAGE_ONLY(env), jclass) {
    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    if (!myGameHolder->game)
        return;
    LOGE_IF(env != myGameHolder->gameEnv, "Incoherent JNIEnv " << env << " != " << myGameHolder->gameEnv);
    // kill all music
    theMusicSystem.toggleMute(true);
    myGameHolder->game->togglePause(true);
    pauseTime = TimeUtil::GetTime();
    LOGW("<--" <<  __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_resume
  (JNIEnv *, jclass) {
    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);

    LOGW("<--" <<  __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_startRendering
  (JNIEnv *, jclass) {
     LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
     if (RenderingSystem::GetInstancePointer()) {
        theRenderingSystem.enableRendering();
     }
     LOGW("<--" <<  __FUNCTION__);
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_stopRendering
  (JNIEnv *, jclass) {
     LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
     if (RenderingSystem::GetInstancePointer()) {
        theRenderingSystem.disableRendering();
     }
     LOGW("<--" <<  __FUNCTION__);
}

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    restoreRenderingSystemState
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_initAndReloadTextures
  (JNIEnv *env, jclass) {
    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    if (!myGameHolder->game || !RenderingSystem::GetInstancePointer())
        return;
    INIT_1(assetAPI, render, AssetAPIAndroidImpl)
    myGameHolder->renderEnv = env;

    theRenderingSystem.init();
    theRenderingSystem.reloadTextures();
    theRenderingSystem.enableRendering();

    LOGW("<--" <<  __FUNCTION__);
}


JNIEXPORT jboolean JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_willConsumeBackEvent
  (JNIEnv *, jclass) {

    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    bool res = ((myGameHolder && myGameHolder->game) ? myGameHolder->game->willConsumeBackEvent() : false);
    LOGW("<--" <<  __FUNCTION__);
    return res;
}

JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_back
  (JNIEnv *LOG_USAGE_ONLY(env), jclass) {
    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    if (!myGameHolder->game)
        return;
    LOGE_IF(env != myGameHolder->gameEnv, "Incoherent JNIEnv " << env << " != " << myGameHolder->gameEnv);
    myGameHolder->game->backPressed();
    LOGW("<--" <<  __FUNCTION__);
}

/*
 * Class:     net_damsy_soupeaucaillou_SacJNILib
 * Method:    handleInputEvent
 * Signature: (JIFF)V
 */
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_handleInputEvent
  (JNIEnv *, jclass, jint evt, jfloat x, jfloat y, jint pointerIndex) {
    if (myGameHolder == 0)
        return;

    GameHolder::__input& input = myGameHolder->input[pointerIndex];

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
  (JNIEnv *env, jclass) {
    LOGW("-->" <<  __FUNCTION__ << ':' << myGameHolder);
    jbyteArray jb = 0;
    if (myGameHolder == 0)
        return jb;

    uint8_t* state;
    int size = myGameHolder->game->saveState(&state);

    if (size) {
        jb = env->NewByteArray(size);
        env->SetByteArrayRegion(jb, 0, size, (jbyte*)state);
        LOGW("Serialized state size: " << size);
    }

    LOGW("<--" <<  __FUNCTION__);
    return jb;
}

JNIEXPORT jboolean JNICALL Java_net_damsy_soupeaucaillou_SacJNILib_isAPIRequired
    (JNIEnv*, jclass, jint apiHash) {
    if (myGameHolder == 0)
        return false;

    switch (apiHash) {
        case 1106735596 /* AssetAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::Asset);
        case 597937595 /* CommunicationAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::Communication);
        case 1581172321 /* LocalizeAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::Localize);
        case 2119627088 /* OpenURLAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::OpenURL);
        case 1105547106 /* SoundAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::Sound);
        case 419223939 /* StorageAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::Storage);
        case 2113329533 /* VibrateAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::Vibrate);
        case -1325525035 /* WWWAPI */:
            return myGameHolder->game->wantsAPI(ContextAPI::WWW);
        default:
            LOGW("Unknown API '" << apiHash << "'. Defaulting to: game do not want to enable it");
            return false;
    }

}

#ifdef __cplusplus
}
#endif
#endif
