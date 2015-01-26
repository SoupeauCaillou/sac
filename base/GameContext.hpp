#pragma once

/* Common interface for all wrappers */
struct APIWrapperI {
    /* APIWrapperI(ContextAPI::Enum pType) : _type(pType) {} */
    virtual ~APIWrapperI() {}

    /*ContextAPI::Enum type() const { return _type;}
    ContextAPI::Enum _type;*/
};

/* Subclass - templated on the API (eg: AdAPI) */
template<class T>
struct APIWrapper : public APIWrapperI {
    APIWrapper<T>(/*ContextAPI::Enum pType*/) : APIWrapperI(/*pType*/) {  }
    virtual ~APIWrapper<T>() {}

    virtual T* as() = 0;

};

/* Sub-sub class - templated on both the API and the implementation
    (eg: T=AssetAPIAndroidImpl and U=AssetAPI */
template<class T, class U>
struct APIWrapperImpl : public APIWrapper<U> {
    APIWrapperImpl<T, U>(/* ContextAPI::Enum pType */ ) : APIWrapper<U>(/*pType*/), ptr(0) {  }
    ~APIWrapperImpl<T, U>() { delete this->ptr; this->ptr = 0; }

    U* as() { if (!ptr) ptr = new T; return ptr; }
    operator T*() { if (!ptr) ptr = new T; return ptr; }
    T* ptr;
};

/* Boilerplate, which I'd love to remove (could be generated using cmake though) */
#include "api/AdAPI.h"
template<> inline ContextAPI::Enum typeToEnum<AdAPI>() { return ContextAPI::Ad; }
#include "api/AssetAPI.h"
template<> inline ContextAPI::Enum typeToEnum<AssetAPI>() { return ContextAPI::Asset; }
#include "api/CommunicationAPI.h"
template<> inline ContextAPI::Enum typeToEnum<CommunicationAPI>() { return ContextAPI::Communication; }
#include "api/ExitAPI.h"
template<> inline ContextAPI::Enum typeToEnum<ExitAPI>() { return ContextAPI::Exit; }
#include "api/GameCenterAPI.h"
template<> inline ContextAPI::Enum typeToEnum<GameCenterAPI>() { return ContextAPI::GameCenter; }
#include "api/InAppPurchaseAPI.h"
template<> inline ContextAPI::Enum typeToEnum<InAppPurchaseAPI>() { return ContextAPI::InAppPurchase; }
#include "api/JoystickAPI.h"
template<> inline ContextAPI::Enum typeToEnum<JoystickAPI>() { return ContextAPI::Joystick; }
#include "api/KeyboardInputHandlerAPI.h"
template<> inline ContextAPI::Enum typeToEnum<KeyboardInputHandlerAPI>() { return ContextAPI::KeyboardInputHandler; }
#include "api/LocalizeAPI.h"
template<> inline ContextAPI::Enum typeToEnum<LocalizeAPI>() { return ContextAPI::Localize; }
#include "api/MusicAPI.h"
template<> inline ContextAPI::Enum typeToEnum<MusicAPI>() { return ContextAPI::Music; }
#include "api/NetworkAPI.h"
template<> inline ContextAPI::Enum typeToEnum<NetworkAPI>() { return ContextAPI::Network; }
#include "api/OpenURLAPI.h"
template<> inline ContextAPI::Enum typeToEnum<OpenURLAPI>() { return ContextAPI::OpenURL; }
#include "api/SoundAPI.h"
template<> inline ContextAPI::Enum typeToEnum<SoundAPI>() { return ContextAPI::Sound; }
#include "api/StorageAPI.h"
template<> inline ContextAPI::Enum typeToEnum<StorageAPI>() { return ContextAPI::Storage; }
#include "api/StringInputAPI.h"
template<> inline ContextAPI::Enum typeToEnum<StringInputAPI>() { return ContextAPI::StringInput; }
#include "api/VibrateAPI.h"
template<> inline ContextAPI::Enum typeToEnum<VibrateAPI>() { return ContextAPI::Vibrate; }
#include "api/WWWAPI.h"
template<> inline ContextAPI::Enum typeToEnum<WWWAPI>() { return ContextAPI::WWW; }
