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



#include "MusicSystem.h"

#include <base/Log.h>

#if SAC_EMSCRIPTEN
#include <SDL_mixer.h>
#include <sstream>
#else
#include "util/OggDecoder.h"
#endif

#if SAC_LINUX
#include <linux/sched.h>
#endif

#include <glm/glm.hpp>

#define SILENCE_AT_BEGINNING_SEC 1.0f

INSTANCE_IMPL(MusicSystem);

MusicSystem::MusicSystem() : ComponentSystemImpl<MusicComponent>("Music"), muted(true), assetAPI(0) {
    /* nothing saved */
    MusicComponent sc;

    componentSerializer.add(new Property<int>("music", OFFSET(music, sc)));
    componentSerializer.add(new Property<int>("loop_next", OFFSET(loopNext, sc)));
    componentSerializer.add(new Property<int>("previous_ending", OFFSET(previousEnding, sc)));

    componentSerializer.add(new Property<float>("loop_at", OFFSET(loopAt, sc), 0.001f));
    componentSerializer.add(new Property<int>("position_i", OFFSET(positionI, sc)));
    componentSerializer.add(new Property<float>("fade_out", OFFSET(fadeOut, sc), 0.001f));
    componentSerializer.add(new Property<float>("fade_in", OFFSET(fadeIn, sc), 0.001f));
    componentSerializer.add(new Property<float>("volume", OFFSET(volume, sc), 0.001f));
    componentSerializer.add(new Property<bool>("looped", OFFSET(looped, sc)));
    componentSerializer.add(new Property<bool>("paused", OFFSET(paused, sc)));
    componentSerializer.add(new StringProperty("auto_loop_name", OFFSET(autoLoopName, sc)));
    componentSerializer.add(new Property<int>("control", OFFSET(control, sc)));
}

MusicSystem::~MusicSystem() {
    LOGV(1, "MusicSystem uninitinalized");
    for (std::map<MusicRef, MusicInfo>::iterator it=musics.begin(); it!=musics.end(); ++it) {
        OggDecoder::release(it->second.handle);
    }
    musics.clear();

    for (std::map<std::string, FileBuffer>::iterator it=name2buffer.begin(); it!=name2buffer.end(); ++it) {
        delete[] it->second.data;
    }
    name2buffer.clear();
}

void MusicSystem::init() {
    muted = false;
    nextValidRef = 1;
}

void MusicSystem::clearAndRemoveInfo(MusicRef ref) {
    if (ref == InvalidMusicRef)
        return;

    std::map<MusicRef, MusicInfo>::iterator it = musics.find(ref);
    if (it == musics.end()) {
        LOGE("Weird, cannot find: " << ref << " music ref");
    } else {
        if (it->second.handle)
            OggDecoder::release(it->second.handle);
        musics.erase(it);
    }
}

void MusicSystem::stopMusic(MusicComponent* m) {
    for (int i=0; i<2; i++) {
        if (m->opaque[i]) {
            musicAPI->stopPlayer(m->opaque[i]);
            musicAPI->deletePlayer(m->opaque[i]);
            m->opaque[i] = 0;
            m->positionI = 0;
        }
    }
    if (m->music != InvalidMusicRef)
        clearAndRemoveInfo(m->music);
    if (m->loopNext != InvalidMusicRef)
        clearAndRemoveInfo(m->loopNext);
    if (m->previousEnding != InvalidMusicRef)
        clearAndRemoveInfo(m->previousEnding);
    m->music = m->loopNext = m->previousEnding = InvalidMusicRef;
    m->currentVolume = 0;
}

void MusicSystem::DoUpdate(float dt) {
    if (muted) {
        FOR_EACH_COMPONENT(Music, m)
            stopMusic(m);
        END_FOR_EACH()
        return;
    }

    FOR_EACH_ENTITY_COMPONENT(Music, entity, m)
        m->looped = false;

        if (m->loopNext == InvalidMusicRef && !m->autoLoopName.empty()) {
            LOGI("Music '" << theEntityManager.entityName(entity) << "': prepare next loop '" << m->autoLoopName << "'");
            m->loopNext = loadMusicFile(m->autoLoopName);
        }

        // Music is not started and is startable => launch opaque[0] player
        if (m->control == MusicControl::Play && m->music != InvalidMusicRef && !m->opaque[0]) {
            // start
            m->opaque[0] = startOpaque(m, m->music, m->master, 0);
            if (m->opaque[1]) {
                musicAPI->stopPlayer(m->opaque[1]);
                musicAPI->deletePlayer(m->opaque[1]);
                clearAndRemoveInfo(m->loopNext);
                clearAndRemoveInfo(m->previousEnding);
                m->loopNext = m->previousEnding = InvalidMusicRef;
            }
            m->opaque[1] = 0;
        } else if (m->control == MusicControl::Stop && m->opaque[0]) {
            if (m->fadeOut > 0) {
                const float step = dt / m->fadeOut;
                if (m->volume > step) {
                    m->volume -= step;
                } else {
                    stopMusic(m);
                }
            } else {
                stopMusic(m);
            }

        } else if (m->control == MusicControl::Pause && m->opaque[0]) {
            musicAPI->pausePlayer(m->opaque[0]);
            if (m->opaque[1]) {
                musicAPI->pausePlayer(m->opaque[1]);
            }
            m->paused = true;
            continue;
        }

        // playing
        if (m->opaque[0]) {
            if (m->paused && m->control == MusicControl::Play) {
                musicAPI->startPlaying(m->opaque[0], 0, 0);
            }
            if (m->fadeIn > 0 && m->currentVolume < m->volume) {
                const float step = dt / m->fadeIn;
                m->currentVolume += step;
                m->currentVolume = glm::min(m->currentVolume, m->volume);
                musicAPI->setVolume(m->opaque[0], m->currentVolume);
            } else {
                m->fadeIn = 0;
                if (m->currentVolume != m->volume) {
                    // LOGW("clear fade in ? %.2f - current: %.2f - volume: %.2f", m->fadeIn, m->currentVolume, m->volume);
                    musicAPI->setVolume(m->opaque[0], m->volume);
                    m->currentVolume = m->volume;
                }
            }

            // can queue more data ?
            feed(m->opaque[0], m->music, dt);

            m->positionI = musicAPI->getPosition(m->opaque[0]);

            LOGE_IF (m->music == InvalidMusicRef, "Invalid music ref: " << m->music);

        #if ! SAC_EMSCRIPTEN
            int sampleRate0 = musics[m->music].sampleRate;
            if ((m->music != InvalidMusicRef && m->positionI >= musics[m->music].nbSamples) || !musicAPI->isPlaying(m->opaque[0]))
        #else
            if (!musicAPI->isPlaying(m->opaque[0]))
        #endif
            {
                LOGI("(music) " << m << " Player 0 has finished (isPlaying:" << musicAPI->isPlaying(m->opaque[0]) << " pos:" << m->positionI << " m->music:" << m->music);
                m->positionI = 0;
                musicAPI->deletePlayer(m->opaque[0]);
                m->opaque[0] = 0;
                // remove m->music from musics
                clearAndRemoveInfo(m->music);
                m->music = InvalidMusicRef;
            }

            // if [0] is valid, and [1] not, and [0] can loop
            if (m->opaque[0] && !m->opaque[1] && m->loopNext != InvalidMusicRef) {
                bool loop = false;
                if (m->master) {
                    loop = m->master->looped;
                } else {
                    #if SAC_EMSCRIPTEN
                        loop = ((m->loopAt > 0) & !musicAPI->isPlaying(m->opaque[0]));
                    #else
                        loop = ((m->loopAt > 0) & (m->positionI >= SEC_TO_SAMPLES(m->loopAt, sampleRate0)));
                    #endif
                }

                if (loop) {
                    #if ! SAC_EMSCRIPTEN
                        LOGV(1, "(music) " << m << " Begin loop (" << m->positionI << " >= " << SEC_TO_SAMPLES(m->loopAt, sampleRate0) << ") - m->music:" << m->music << " becomes loopNext:" << m->loopNext << " [master=" << m->master << ']');
                    #endif

                    m->looped = true;
                    m->opaque[1] = m->opaque[0];
                    // memorize ending music
                    m->previousEnding = m->music;
                    // start new loop
                    m->music = m->loopNext;
                    // clear new loop selection
                    m->loopNext = InvalidMusicRef;

                    if (m->master) {
                        m->opaque[0] = startOpaque(m, m->music, m->master, 0);
                    } else {
#if ! SAC_EMSCRIPTEN
                        int offset = m->positionI - SEC_TO_SAMPLES(m->loopAt, sampleRate0);
#else
                        int offset = 0;
#endif
                        m->opaque[0] = startOpaque(m, m->music, 0, offset);
                    }

                    m->positionI = musicAPI->getPosition(m->opaque[0]);
                }
            }
        }

        if (m->opaque[1]) {
            if (m->paused && m->control == MusicControl::Play) {
                musicAPI->startPlaying(m->opaque[1], 0, 0);
            }
            LOGE_IF(m->previousEnding == InvalidMusicRef, "Invalid previousEnding " << m->previousEnding);
            if (m->currentVolume != m->volume) {
                musicAPI->setVolume(m->opaque[1], m->volume);
            }

            feed(m->opaque[1], m->previousEnding, dt);
            if ((m->previousEnding != InvalidMusicRef && musicAPI->getPosition(m->opaque[1]) >= musics[m->previousEnding].nbSamples) || !musicAPI->isPlaying(m->opaque[1])) {
                musicAPI->deletePlayer(m->opaque[1]);
                m->opaque[1] = 0;
                // remove m->loopNext from musics
                clearAndRemoveInfo(m->previousEnding);
                m->previousEnding = InvalidMusicRef;
                LOGV(1, "(music) " << m << " Player 1 has finished");
            }
        }

        if (!m->opaque[0] && m->master && m->loopNext != InvalidMusicRef && m->control == MusicControl::Play) {
            if (m->master->looped) {
                LOGV(1, "(music) Restarting because master has looped (current: " << m->music << " -> next: " << m->loopNext << ')');
                m->music = m->loopNext;
                if (m->opaque[1]) {
                    LOGW("(music) Weird, shouldn't happen");
                    musicAPI->deletePlayer(m->opaque[1]);
                    clearAndRemoveInfo(m->previousEnding);
                    m->previousEnding = InvalidMusicRef;
                    m->opaque[1] = 0;
                }
                m->loopNext = InvalidMusicRef;
                m->opaque[0] = startOpaque(m, m->music, m->master, 0);
            }
        }
        if (m->control == MusicControl::Play) {
            m->paused = false;
        }
    }
}

void MusicSystem::feed(OpaqueMusicPtr* ptr, MusicRef m, float dt) {
    LOGE_IF (m == InvalidMusicRef, "Cannot feed, invalid music ref");
    if (musics.find(m) == musics.end()) {
        LOGE("Achtung, musicref : " << m << " not found");
        return;
    }

    MusicInfo& info = musics[m];

    info.queuedDuration -= dt;

    if (info.queuedDuration < 0.5) {
        int avail = 0;

        while (info.queuedDuration < 0.75 && (avail = OggDecoder::availableSamples(info.handle))) {
            short* b = new short[avail];
            int count = OggDecoder::readSamples(info.handle, avail, b);
            musicAPI->queueMusicData(ptr, b, count, info.sampleRate);

            info.queuedDuration += count / (float)info.sampleRate;
        }
    }
}

OpaqueMusicPtr* MusicSystem::startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset) {
    LOGE_IF (r == InvalidMusicRef, "Invalid music ref");

    MusicInfo& info = musics[r];
    if (info.sampleRate <=0) {
        LOGW("Invalid sample rate: " << info.sampleRate);
    }
    OpaqueMusicPtr* ptr = m->opaque[0] = musicAPI->createPlayer(info.sampleRate);

    // init with SILENCE_AT_BEGINNING_SEC seconds of silence
    OggInfo::Values in = OggDecoder::query(info.handle);
    int samples_1_sec = in.sampleRate * in.numChannels * SILENCE_AT_BEGINNING_SEC;
    short* buf = new short[samples_1_sec];
    memset(buf, 0, samples_1_sec * sizeof(short));

    LOGV(1, "Start playgin");
    musicAPI->queueMusicData(ptr, buf, samples_1_sec, in.sampleRate);
    info.queuedDuration = 1;

    m->volume = 1;

    // set volume
    if (m->fadeIn > 0) {
        musicAPI->setVolume(ptr, 0);
        m->currentVolume = 0;
        LOGV(1, "(music) volume - Start with fading: " << m->fadeIn << " - " << m->volume);
    } else {
        LOGV(1, "(music) volume - Start without fading: " << m->fadeIn << " - " << m->volume);
        musicAPI->setVolume(ptr, m->volume);
        m->currentVolume = m->volume;
    }

    musicAPI->startPlaying(ptr, master ? master->opaque[0] : 0, offset);
    return ptr;
}

void MusicSystem::toggleMute(bool enable) {
    if (enable && !muted) {
        muted = true;
        FOR_EACH_COMPONENT(Music, m)
            stopMusic(m);
        END_FOR_EACH()
    } else if (!enable && muted) {
        muted = false;
    }
}

MusicRef MusicSystem::loadMusicFile(const std::string& assetName) {
    LOGV(1, "loadMusicFile " << assetName);

    LOGF_IF(!assetAPI, "Asked to load a music file but invalid assetAPI given. Did you init MusicSystem?");

    PROFILE("Music", "loadMusicFile", BeginEvent);

    FileBuffer b;
    if (name2buffer.find(assetName) == name2buffer.end()) {
        PROFILE("Music", "loadAsset", BeginEvent);
        b = assetAPI->loadAsset(assetName);
        PROFILE("Music", "loadAsset", EndEvent);
        if (!b.data) {
            LOGE("Unable to load " << assetName);
            PROFILE("Music", "loadMusicFile", EndEvent);
            return InvalidMusicRef;
        }
        name2buffer[assetName] = b;
    } else {
        b = name2buffer[assetName];
    }
    MusicInfo info;

    info.handle = OggDecoder::load(&b, OggOption::Async);
    OggInfo::Values in = OggDecoder::query(info.handle);

    info.totalTime = in.durationSeconds + SILENCE_AT_BEGINNING_SEC;
    info.nbSamples = in.durationSamples + in.sampleRate * SILENCE_AT_BEGINNING_SEC;
    info.sampleRate = in.sampleRate;
    info.queuedDuration = 0;
    info.numChannels = in.numChannels;
    info.toRemove = false;
    LOGV(1, "(music) File: " << assetName << " / rate: " << info.sampleRate << " duration: " << info.totalTime << " nbSample: " << info.nbSamples << " -> " << nextValidRef);
    PROFILE("Music", "mutex-section", BeginEvent);

    musics[nextValidRef] = info;

    PROFILE("Music", "mutex-section", EndEvent);

    PROFILE("Music", "loadMusicFile", EndEvent);
    return nextValidRef++;
}

