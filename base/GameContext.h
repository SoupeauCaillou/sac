/*
    This file is part of sac.

    @author Soupe au Caillou

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
#pragma once

class AdAPI;
class AssetAPI;
class CommunicationAPI;
class ExitAPI;
class GameCenterAPI;
class KeyboardInputHandlerAPI;
class LocalizeAPI;
class MusicAPI;
class NetworkAPI;
class SoundAPI;
class StorageAPI;
class StringInputAPI;
class VibrateAPI;
class WWWAPI;

namespace ContextAPI {
    enum Enum {
        Ad,
        Asset,
        Communication,
        Exit,
        GameCenter,
        KeyboardInputHandler,
        Localize,
        Music,
        Network,
        Sound,
        Storage,
        StringInput,
        Vibrate,
        WWW,
    };
}

struct GameContext {
    GameContext() :
        adAPI(0),
        assetAPI(0),
        communicationAPI(0),
        exitAPI(0),
        gameCenterAPI(0),
        keyboardInputHandlerAPI(0),
        localizeAPI(0),
        musicAPI(0),
        networkAPI(0),
        soundAPI(0),
        storageAPI(0),
        stringInputAPI(0),
        vibrateAPI(0),
        wwwAPI(0) {}

    AdAPI* adAPI;
    AssetAPI* assetAPI;
    CommunicationAPI* communicationAPI;
    ExitAPI* exitAPI;
    GameCenterAPI* gameCenterAPI;
    KeyboardInputHandlerAPI* keyboardInputHandlerAPI;
    LocalizeAPI* localizeAPI;
    MusicAPI* musicAPI;
    NetworkAPI* networkAPI;
    SoundAPI* soundAPI;
    StorageAPI* storageAPI;
    StringInputAPI* stringInputAPI;
    VibrateAPI* vibrateAPI;
    WWWAPI* wwwAPI;
};
