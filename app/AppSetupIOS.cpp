#if SAC_IOS
#include "AppSetup.h"

#include "base/Game.h"
#include "base/GameContext.h"
#include "base/TouchInputManager.h"
#include "base/TimeUtil.h"

#include "api/AdAPI.h"
#include "api/linux/AssetAPILinuxImpl.h"
#include "api/linux/CommunicationAPILinuxImpl.h"
#include "api/linux/ExitAPILinuxImpl.h"
#include "api/default/LocalizeAPITextImpl.h"
#include "api/linux/MusicAPILinuxOpenALImpl.h"
#include "api/linux/OpenURLAPILinuxImpl.h"
#include "api/linux/SoundAPILinuxOpenALImpl.h"
#include "api/linux/VibrateAPILinuxImpl.h"
#include "api/linux/WWWAPIcURLImpl.h"
#include "api/ios/IOSTouchState.h"
#include "api/default/GameCenterAPIDebugImpl.h"
#include "api/default/SqliteStorageAPIImpl.h"
#include "api/default/AdAPIDebugImpl.h"
#include "api/default/InAppPurchaseAPIDebugImpl.h"

#include "systems/RenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"
#include "systems/RenderingSystem.h"

#include "base/Common.h"

GameContext* setupGameContext(Game* game) {
    /////////////////////////////////////////////////////
    // Game context initialisation
    LOGV(1, "Initialize APIs");
    GameContext* ctx = new GameContext;
    if (game->wantsAPI(ContextAPI::Ad))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::Asset) || true)
        ctx->assetAPI = new AssetAPILinuxImpl();
    if (game->wantsAPI(ContextAPI::Communication))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::Exit))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::GameCenter))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::InAppPurchase))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::Localize))
        ctx->localizeAPI = new LocalizeAPITextImpl();
    if (game->wantsAPI(ContextAPI::Music))
        ctx->musicAPI = new MusicAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::OpenURL))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::Sound))
        ctx->soundAPI = new SoundAPILinuxOpenALImpl();
    if (game->wantsAPI(ContextAPI::Storage))
       ctx->storageAPI = new SqliteStorageAPIImpl();
    if (game->wantsAPI(ContextAPI::StringInput))
        LOGW("TODO");
    if (game->wantsAPI(ContextAPI::Vibrate))
        LOGW("TODO");
#if SAC_HAVE_CURL
    if (game->wantsAPI(ContextAPI::WWW))
        ctx->wwwAPI = new WWWAPIcURLImpl();
#endif

    return ctx;
}

int setupEngine(Game* game, const struct SetupInfo* info) {
#if SAC_ENABLE_LOG
    logLevel = LogVerbosity::VERBOSE1;
#endif

    TimeUtil::Init();

    GameContext* ctx = setupGameContext(game);

    theTouchInputManager.setNativeTouchStatePtr(new IOSTouchState());
    theRenderingSystem.assetAPI = ctx->assetAPI;

    static_cast<AssetAPILinuxImpl*>(ctx->assetAPI)->init(info->name);

    if (game->wantsAPI(ContextAPI::Music)) {
        theMusicSystem.musicAPI = ctx->musicAPI;
        theMusicSystem.assetAPI = ctx->assetAPI;
        static_cast<MusicAPILinuxOpenALImpl*>(ctx->musicAPI)->init();
        theMusicSystem.init();
    }
    if (game->wantsAPI(ContextAPI::Sound)) {
        theSoundSystem.soundAPI = ctx->soundAPI;
        static_cast<SoundAPILinuxOpenALImpl*>(ctx->soundAPI)->init(ctx->assetAPI, game->wantsAPI(ContextAPI::Music));
        theSoundSystem.init();
    }
    if (game->wantsAPI(ContextAPI::Localize)) {
        static_cast<LocalizeAPITextImpl*>(ctx->localizeAPI)->init(ctx->assetAPI);
    }

    /////////////////////////////////////////////////////
    // Init game
    LOGV(1, "Initialize sac & game");
    game->setGameContexts(ctx, ctx);
    sac::setResolution(info->resolution.x, info->resolution.y);
    game->sacInit();

    game->init(NULL, 0);

    theRenderingSystem.enableRendering();

    setlocale( LC_ALL, "" );
    setlocale( LC_NUMERIC, "C" );
    return 0;
}

int tickEngine(Game* game) {
    game->step();
    game->render();
    return 0;
}
#endif
