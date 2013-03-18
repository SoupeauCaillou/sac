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
class NameInputAPI;
class NetworkAPI;
class SoundAPI;
class SuccessAPI;
class VibrateAPI;

#define CONTEXT_WANT_AD_API         0x1 << 0
#define CONTEXT_WANT_ASSET_API      0x1 << 1
#define CONTEXT_WANT_COMM_API       0x1 << 2
#define CONTEXT_WANT_EXIT_API       0x1 << 3
#define CONTEXT_WANT_LOCALIZE_API   0x1 << 4
#define CONTEXT_WANT_MUSIC_API      0x1 << 5
#define CONTEXT_WANT_NAME_INPUT_API 0x1 << 6
#define CONTEXT_WANT_NETWORK_API    0x1 << 7
#define CONTEXT_WANT_SOUND_API      0x1 << 8
#define CONTEXT_WANT_SUCCESS_API    0x1 << 9
#define CONTEXT_WANT_VIBRATE_API    0x1 << 10


struct GameContext {
    AdAPI* adAPI;
    AssetAPI* assetAPI;
    CommunicationAPI* communicationAPI;
    ExitAPI* exitAPI;
    LocalizeAPI* localizeAPI;
    MusicAPI* musicAPI;
    NameInputAPI* nameInputAPI;
    NetworkAPI* networkAPI;
    SoundAPI* soundAPI;
    SuccessAPI* successAPI;
    VibrateAPI* vibrateAPI;
};