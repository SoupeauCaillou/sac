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
class LocalizeAPI;
class MusicAPI;
class KeyboardInputHandlerAPI;
class NetworkAPI;
class SoundAPI;
class StorageAPI;
class SuccessAPI;
class VibrateAPI;

namespace ContextAPI {
    enum Enum {
        Ad,
        Asset,
        Communication,
        Exit,
        KeyboardInputHandler,
        Localize,
        Music,
        Network,
        Sound,
        Storage,
        Success,
        Vibrate
    };
}

struct GameContext {
    GameContext() : adAPI(0),
        assetAPI(0), communicationAPI(0),
        exitAPI(0), keyboardInputHandlerAPI(0),
        localizeAPI(0), musicAPI(0),
        networkAPI(0), soundAPI(0),
        storageAPI(0),
        successAPI(0), vibrateAPI(0) {}
    AdAPI* adAPI;
    AssetAPI* assetAPI;
    CommunicationAPI* communicationAPI;
    ExitAPI* exitAPI;
    KeyboardInputHandlerAPI* keyboardInputHandlerAPI;
    LocalizeAPI* localizeAPI;
    MusicAPI* musicAPI;
    NetworkAPI* networkAPI;
    SoundAPI* soundAPI;
    StorageAPI* storageAPI;
    SuccessAPI* successAPI;
    VibrateAPI* vibrateAPI;
};
