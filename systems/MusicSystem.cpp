#include "MusicSystem.h"
#ifdef SAC_MUSIC_VISU
#include "TextRenderingSystem.h"
#include "RenderingSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"
#endif
#include "base/CircularBuffer.h"

#ifdef SAC_ANDROID
#define assert(x) x
#endif

#ifdef SAC_EMSCRIPTEN
	#include <SDL/SDL_mixer.h>
	#include <sstream>
#else
	#ifdef SAC_ANDROID
		#include "tremor/ivorbisfile.h"
	#else
		#include <vorbis/vorbisfile.h>
	#endif
#endif

#ifdef SAC_LINUX
	#include <linux/sched.h>
#endif

#include "base/MathUtil.h"

INSTANCE_IMPL(MusicSystem);

MusicSystem::MusicSystem() : ComponentSystemImpl<MusicComponent>("Music"), assetAPI(0) {
    /* nothing saved */
}

MusicSystem::~MusicSystem() {
    #ifndef SAC_EMSCRIPTEN
    runDecompLoop = false;
    cond.notify_all();
    oggDecompressionThread.join();
    LOGI("MusicSystem uninitinalized")
    #endif
    #ifndef SAC_EMSCRIPTEN
    for (std::map<MusicRef, MusicInfo>::iterator it=musics.begin(); it!=musics.end(); ++it) {
        delete it->second.buffer;
        ov_clear(it->second.ovf);
        delete it->second.ovf;
    #else
    for (std::map<MusicRef, Mix_Chunk*>::iterator it=musics.begin(); it!=musics.end(); ++it) {
        // delete ...
    #endif
    }
    musics.clear();
    #ifndef SAC_EMSCRIPTEN
    for (std::map<std::string, FileBuffer>::iterator it=name2buffer.begin(); it!=name2buffer.end(); ++it) {
        delete[] it->second.data;
    }
    name2buffer.clear();
    #endif
}

#ifndef SAC_EMSCRIPTEN
static void* _startOggThread(void* arg) {
    static_cast<MusicSystem*>(arg)->oggDecompRunLoop();
    return 0;
}
#endif

void MusicSystem::init() {
    muted = false;
    nextValidRef = 1;

#ifndef SAC_EMSCRIPTEN
    oggDecompressionThread = std::thread(_startOggThread, this);
#endif
}

void MusicSystem::clearAndRemoveInfo(MusicRef ref) {
#ifndef SAC_EMSCRIPTEN
    if (ref == InvalidMusicRef)
        return;
    mutex.lock();
    std::map<MusicRef, MusicInfo>::iterator it = musics.find(ref);
    if (it == musics.end()) {
        LOGW("Weird, cannot find: " << ref << " music ref")
    } else {
        // LOGW("Delayed erase music ref: %d", ref);
        it->second.toRemove = true;
    }
    mutex.unlock();
    cond.notify_all();
#else
	Mix_FreeChunk(musics[ref]);
	musics.erase(ref);
#endif
}

void MusicSystem::unloadMusic(MusicRef ref) {
	clearAndRemoveInfo(ref);
}

void MusicSystem::stopMusic(MusicComponent* m) {
    for (int i=0; i<2; i++) {
        if (m->opaque[i]) {
            musicAPI->stopPlayer(m->opaque[i]);
            musicAPI->deletePlayer(m->opaque[i]);
            m->opaque[i] = 0;
            m->positionI = 0;
            #ifdef SAC_MUSIC_VISU
            m->positionF = 0;
            #endif
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
        }
        return;
    }

    FOR_EACH_COMPONENT(Music, m)
		m->looped = false;

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
		        m->currentVolume = MathUtil::Min(m->currentVolume, m->volume);
		        musicAPI->setVolume(m->opaque[0], m->currentVolume);
	        } else {
	        	m->fadeIn = 0;
	        	if (m->currentVolume != m->volume) {
		      		// LOGW("clear fade in ? %.2f - current: %.2f - volume: %.2f", m->fadeIn, m->currentVolume, m->volume);
	            	musicAPI->setVolume(m->opaque[0], m->volume);
	            	m->currentVolume = m->volume;
	        	}
            }
            // need to queue more data ?
            #ifndef SAC_EMSCRIPTEN
            feed(m->opaque[0], m->music, 0, dt);
            #endif
            m->positionI = musicAPI->getPosition(m->opaque[0]);

            assert (m->music != InvalidMusicRef);
            #ifndef SAC_EMSCRIPTEN
            int sampleRate0 = musics[m->music].sampleRate;
            if ((m->music != InvalidMusicRef && m->positionI >= musics[m->music].nbSamples) || !musicAPI->isPlaying(m->opaque[0])) {
	        #else
	        if (!musicAPI->isPlaying(m->opaque[0])) {
	        #endif
	            LOGV(1, "(music) " << m << " Player 0 has finished (isPlaying:" << musicAPI->isPlaying(m->opaque[0]) << " pos:" << m->positionI << " m->music:" << m->music)
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
	                #ifndef SAC_EMSCRIPTEN
                    loop = ((m->loopAt > 0) & (m->positionI >= SEC_TO_SAMPLES(m->loopAt, sampleRate0)));
                    #else
                    loop = ((m->loopAt > 0) & !musicAPI->isPlaying(m->opaque[0]));
                    #endif
                }

                if (loop) {
                    #ifndef SAC_EMSCRIPTEN
                    LOGV(1, "(music) " << m << " Begin loop (" << m->positionI << " >= " << SEC_TO_SAMPLES(m->loopAt, sampleRate0) << ") - m->music:" << m->music << " becomes loopNext:" << m->loopNext << " [master=" << m->master << ']')
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
                        #ifndef SAC_EMSCRIPTEN
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
            assert(m->previousEnding != InvalidMusicRef);
            if (m->currentVolume != m->volume) {
                musicAPI->setVolume(m->opaque[1], m->volume);
            }
            #ifndef SAC_EMSCRIPTEN
            feed(m->opaque[1], m->previousEnding, 0, dt);
            if ((m->previousEnding != InvalidMusicRef && musicAPI->getPosition(m->opaque[1]) >= musics[m->previousEnding].nbSamples) || !musicAPI->isPlaying(m->opaque[1])) {
            #else
            if (!musicAPI->isPlaying(m->opaque[1])) {
            #endif
                musicAPI->deletePlayer(m->opaque[1]);
                m->opaque[1] = 0;
                // remove m->loopNext from musics
                clearAndRemoveInfo(m->previousEnding);
                m->previousEnding = InvalidMusicRef;
                LOGV(1, "(music) " << m << " Player 1 has finished")
            }
        }

        if (!m->opaque[0] && m->master && m->loopNext != InvalidMusicRef && m->control == MusicControl::Play) {
            if (m->master->looped) {
                LOGV(1, "(music) Restarting because master has looped (current: " << m->music << " -> next: " << m->loopNext << ')')
                m->music = m->loopNext;
                if (m->opaque[1]) {
                    LOGW("(music) Weird, shouldn't happen")
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


        #ifdef SAC_MUSIC_VISU
        if (m->music != InvalidMusicRef) {
            m->positionF = m->positionI / (float)musics[m->music].nbSamples;
        }
        #endif
    }

    #ifdef SAC_MUSIC_VISU
    int idx = 0;
    FOR_EACH_ENTITY_COMPONENT(Music, a, rc)
        if (visualisationEntities.find(a) == visualisationEntities.end()) {
            int size = visualisationEntities.size();

            Entity e = theEntityManager.CreateEntity();
            ADD_COMPONENT(e, Rendering);
            RENDERING(e)->color = Color((size % 2), (size % 2), (size % 2), 0.6);
            RENDERING(e)->show = false;
            ADD_COMPONENT(e, Transformation);
            TRANSFORM(e)->size = Vector2(0.5, 1);
            TRANSFORM(e)->z = 0.75;
            ADD_COMPONENT(e, TextRendering);
            TEXT_RENDERING(e)->charHeight = 0.4;
            TEXT_RENDERING(e)->color = Color(0,0,0);
            TEXT_RENDERING(e)->text = "A";
            TEXT_RENDERING(e)->show = false;

            Entity f = theEntityManager.CreateEntity();
            ADD_COMPONENT(f, Rendering);
            RENDERING(f)->color = Color((size % 2), (size % 2), (size % 2), 0.6);
            RENDERING(f)->show = false;
            ADD_COMPONENT(f, Transformation);
            TRANSFORM(f)->size = Vector2(0.5, 1);
            TRANSFORM(f)->z = 0.75;
            ADD_COMPONENT(f, TextRendering);
            TEXT_RENDERING(f)->charHeight = 0.3;
            TEXT_RENDERING(f)->color = Color(0,0,0);
            TEXT_RENDERING(f)->text = "B";

            visualisationEntities[a] = std::make_pair(e, f);
        }
        float VisuWidth = PlacementHelper::GimpWidthToScreen(50);
        Entity e = visualisationEntities[a].first;
        Entity f = visualisationEntities[a].second;
        TRANSFORM(e)->size = Vector2(VisuWidth * 0.5, rc->positionF * PlacementHelper::GimpHeightToScreen(1280));
        TransformationSystem::setPosition(TRANSFORM(e),
            Vector2(
                PlacementHelper::GimpXToScreen(0) + idx * VisuWidth * 2, PlacementHelper::GimpYToScreen(0)),
            TransformationSystem::NW);
         if (rc->previousEnding != InvalidMusicRef) {
            float pF = rc->opaque[1] ? (musicAPI->getPosition(rc->opaque[1]) / (float)musics[rc->previousEnding].nbSamples) : 1;
            TRANSFORM(f)->size = Vector2(VisuWidth * 0.5, pF * PlacementHelper::GimpHeightToScreen(1280));
            TransformationSystem::setPosition(TRANSFORM(f),
            Vector2(
                PlacementHelper::GimpXToScreen(0) + idx * VisuWidth * 2 + TRANSFORM(e)->size.X * 0.5, PlacementHelper::GimpYToScreen(0)),
            TransformationSystem::NW);
            RENDERING(f)->show = rc->opaque[1] ? false : true;
         } else {
            RENDERING(f)->show = true;
         }

         if (rc->music != InvalidMusicRef) {
            if (rc->loopNext != InvalidMusicRef) {
                TEXT_RENDERING(e)->text = musics[rc->music].name + "}" + musics[rc->loopNext].name;
            } else {
                TEXT_RENDERING(e)->text = musics[rc->music].name;
            }
            TEXT_RENDERING(e)->show = false;
         } else if (rc->loopNext != InvalidMusicRef) {
            TEXT_RENDERING(e)->text = "}" + musics[rc->loopNext].name;
            TEXT_RENDERING(e)->show = false;
         } else {
            TEXT_RENDERING(e)->show = true;
         }
         if (rc->previousEnding != InvalidMusicRef) {
            TEXT_RENDERING(f)->text = musics[rc->previousEnding].name;
            TEXT_RENDERING(f)->show = false;
         } else {
            TEXT_RENDERING(f)->show = true;
         }

        if (rc->control == MusicControl::Stop) {
            RENDERING(e)->color = RENDERING(f)->color = Color(0.3, 0, 0, 0.5);
        } else if (rc->opaque[0]) {
            if (rc->loopNext != InvalidMusicRef) {
                RENDERING(e)->color = RENDERING(f)->color = Color(0, 0.8, 0, 0.5);
            } else {
                RENDERING(e)->color = RENDERING(f)->color = Color(0, 0.3, 0.5, 0.5);
            }
        } else {
            RENDERING(e)->color = RENDERING(f)->color = Color(0.9, 0.9, 0, 0.5);
        }
        idx++;
    }
    #endif
}

#ifndef SAC_EMSCRIPTEN
void MusicSystem::oggDecompRunLoop() {
    runDecompLoop = true;

    std::unique_lock<std::mutex> lock(mutex);

    // one static buffer to rule them all
    int8_t tempBuffer[48000 * 2]; // 1 sec * 48Hz * 2 bytes/sample


    while (runDecompLoop) {
        bool roomForImprovement = false;
        PROFILE("Music", "DecompressMusic", BeginEvent);
        for (std::map<MusicRef, MusicInfo>::iterator it=musics.begin(); it!=musics.end(); ) {
            assert(it->first != InvalidMusicRef);
            MusicInfo& info = it->second;

            if (info.toRemove) {
                if (info.ovf) {
                    ov_clear(info.ovf);
                    delete info.ovf;
                }
                if (info.buffer) {
                    LOGV(1, "delete " << info.buffer)
                    delete info.buffer;
                    info.buffer = 0;
                }
                // deallocate nextPcmBuffer to
                std::map<MusicRef, MusicInfo>::iterator jt = it;
                ++it;
                musics.erase(jt);
                // LOGW("------------------------------------- remove %d => %lu", jt->first, musics.size());
                continue;
            } else {
                ++it;
            }

            unsigned int chunkSize = info.pcmBufferSize; //info.buffer->getBufferSize() * 0.5;

            if (info.buffer->writeSpaceAvailable() >= chunkSize) {
             	// LOGW("%d] decompress %d bytes", it->first, chunkSize);
                decompressNextChunk(info.ovf, tempBuffer, chunkSize);
                // LOGW("decomp done");
                info.buffer->write(tempBuffer, chunkSize);
            }
            if (info.buffer->writeSpaceAvailable() >= chunkSize) {
            	roomForImprovement = true;
            }
        }
        PROFILE("Music", "DecompressMusic", EndEvent);
        // release mutex while waiting
        if (!roomForImprovement) {
        	cond.wait(lock);
        	// mutex is auto acquired on wake up
        } else {
            lock.unlock();
            lock.lock();
        }
    }
}
#endif

#ifndef SAC_EMSCRIPTEN
bool MusicSystem::feed(OpaqueMusicPtr* ptr, MusicRef m, int, float dt) {
    assert (m != InvalidMusicRef);
    if (musics.find(m) == musics.end()) {
        LOGW("Achtung, musicref : " << m << " not found")
        return false;
    }

    MusicInfo& info = musics[m];

    dt += info.leftOver;
    float chunkDuration = (info.pcmBufferSize / 2) / (float)info.sampleRate;
    // LOGW("timeDiff: %.3f / chunk: %.3f / %.3f / %d", dt, chunkDuration, info.leftOver, info.buffer->readDataAvailable());

    while (dt >= chunkDuration) {

    // while (musicAPI->needData(ptr, info.sampleRate, false)) {
        int count = info.buffer->readDataAvailable();
        // LOGW("%d) DATA AVAILABLE: %d >= %d ?", m, count, info.pcmBufferSize);
        if (count < info.pcmBufferSize * 3) {
            // wake-up decomp thread via notify
            cond.notify_all();
        }
        if (count >= info.pcmBufferSize) {
            int8_t* b = musicAPI->allocate(info.pcmBufferSize);
            info.buffer->read(b, info.pcmBufferSize);
            musicAPI->queueMusicData(ptr, b, info.pcmBufferSize, info.sampleRate);
            dt -= chunkDuration;
        } else {
            LOGW("Fcuk, not enough data: " << count << " < " << info.pcmBufferSize)
            break;
        }
    }
    info.leftOver = dt;

    return true;
}
#endif

OpaqueMusicPtr* MusicSystem::startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset) {
    assert (r != InvalidMusicRef);
#ifndef SAC_EMSCRIPTEN
    MusicInfo& info = musics[r];
    if (info.sampleRate <=0) {
        LOGW("Invalid sample rate: " << info.sampleRate)
    }
    OpaqueMusicPtr* ptr = m->opaque[0] = musicAPI->createPlayer(info.sampleRate);

    int initialNeededCount = musicAPI->initialPacketCount(ptr);
    info.nbSamples += initialNeededCount * info.pcmBufferSize / 2;
    for (int i=0; i<initialNeededCount; i++) {
        // init with silence pkts
        int8_t* buffer0 = musicAPI->allocate(info.pcmBufferSize);
        memset(buffer0, 0,  info.pcmBufferSize);
        musicAPI->queueMusicData(ptr, buffer0, info.pcmBufferSize, info.sampleRate);
    }
#else
    OpaqueMusicPtr* ptr = m->opaque[0] = musicAPI->createPlayer(0);
    musicAPI->queueMusicData(ptr, static_cast<int8_t*>((void*)musics[r]), 0, 1);
#endif
    m->volume = 1;
    // set volume
    if (m->fadeIn > 0) {
        musicAPI->setVolume(ptr, 0);
        m->currentVolume = 0;
        LOGV(1, "(music) volume - Start with fading: " << m->fadeIn << " - " << m->volume)
    } else {
        LOGV(1, "(music) volume - Start without fading: " << m->fadeIn << " - " << m->volume)
        musicAPI->setVolume(ptr, m->volume);
        m->currentVolume = m->volume;
    }
    //musicAPI->setVolume(ptr, m->volume);
    musicAPI->startPlaying(ptr, master ? master->opaque[0] : 0, offset);
    return ptr;
}

void MusicSystem::toggleMute(bool enable) {
#ifndef SAC_EMSCRIPTEN
    if (enable && !muted) {
        muted = true;
        FOR_EACH_COMPONENT(Music, m)
            stopMusic(m);
        }
    } else if (!enable && muted) {
        muted = false;
    }
#else
    muted = true;
#endif
}

#ifndef SAC_EMSCRIPTEN
static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource);
static int seek_func(void* datasource, ogg_int64_t offset, int whence);
static long int tell_func(void* datasource);
static int close_func(void *datasource);
#endif

struct DataSource {
    uint8_t* datas;
    int size;
    int cursor;
};

MusicRef MusicSystem::loadMusicFile(const std::string& assetName) {
    LOGI("loadMusicFile " << assetName)
    if (!assetAPI)
        return InvalidMusicRef;
    PROFILE("Music", "loadMusicFile", BeginEvent);
    #ifndef SAC_EMSCRIPTEN
    FileBuffer b;
    if (name2buffer.find(assetName) == name2buffer.end()) {
        PROFILE("Music", "loadAsset", BeginEvent);
        b = assetAPI->loadAsset(assetName);
        PROFILE("Music", "loadAsset", EndEvent);
        if (!b.data) {
            LOGE("Unable to load " << assetName)
            PROFILE("Music", "loadMusicFile", EndEvent);
            return InvalidMusicRef;
        }
        name2buffer[assetName] = b;
    } else {
        b = name2buffer[assetName];
    }
    #endif

#ifndef SAC_EMSCRIPTEN
    DataSource* dataSource = new DataSource();
    dataSource->datas = b.data;
    dataSource->size = b.size;
    dataSource->cursor = 0;

    ov_callbacks cb;
    cb.read_func = &read_func;
    cb.seek_func = &seek_func;
    cb.close_func = &close_func;
    cb.tell_func = &tell_func;

    OggVorbis_File* f = new OggVorbis_File();
    PROFILE("Music", "ov_open_callbacks", BeginEvent);
    ov_open_callbacks(dataSource, f, 0, 0, cb);
    PROFILE("Music", "ov_open_callbacks", EndEvent);

    MusicInfo info;
    info.ovf = f;
    info.totalTime = ov_time_total(f, -1) * 0.001 + 1; // hum hum
    vorbis_info* inf = ov_info(f, -1);
    info.toRemove = false;
    info.sampleRate = inf->rate * inf->channels;
    info.pcmBufferSize = musicAPI->pcmBufferSize(info.sampleRate);
    info.nbSamples = ov_pcm_total(f, -1);
    info.leftOver = 0;
    info.buffer = new CircularBuffer(info.pcmBufferSize * 10);
    LOGV(1, "(music) File: " << assetName << " / rate: " << info.sampleRate << " duration: " << info.totalTime << " nbSample: " << info.nbSamples << " -> " << nextValidRef)
    PROFILE("Music", "mutex-section", BeginEvent);
    mutex.lock();
    #ifdef SAC_MUSIC_VISU
    int start = assetName.find("audio/") + 6;
    int end = assetName.find(".ogg");
    info.name = assetName.substr(start, end - start);
    #endif
    musics[nextValidRef] = info;
    // LOGI("================================ ++ %d => %lu", nextValidRef, musics.size());
    mutex.unlock();
    PROFILE("Music", "mutex-section", EndEvent);
    // wake-up decompression thread
    cond.notify_all();
#else
    std::stringstream a;
    a << "assets/" << assetName;
    musics[nextValidRef] = Mix_LoadWAV(a.str().c_str());
    LOGV(1, "Load music file " << a.str() << " -> " <<< musics[nextValidRef])
#endif
    PROFILE("Music", "loadMusicFile", EndEvent);
    return nextValidRef++;
}

#ifndef SAC_EMSCRIPTEN
int MusicSystem::decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize) {
    int bitstream;
    int read = 0;
    while (read < chunkSize) {
        #ifdef SAC_ANDROID
        int n = ov_read(file, (char*) &data[read], chunkSize - read, &bitstream);
        #else
        int n = ov_read(file, (char*) &data[read], chunkSize - read, 0, 2, 1, &bitstream);
        #endif
        if (n == 0) {
            // LOGI("%p] EOF (read: %d/%d)", file, read, chunkSize);
            // EOF
            break;
        } else if (n < 0) {
            LOGW("Error in vorbis read: " << n)
            break;
        } else {
            read += n;
        }
    }

    if (read < chunkSize) {
        // LOGI("%p] Producing %d/%d bytes of silence", file, chunkSize - read, chunkSize);
        memset(&data[read], 0, chunkSize - read);
    }
    return chunkSize;
}
#endif

#ifndef SAC_EMSCRIPTEN
static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    DataSource* src = static_cast<DataSource*> (datasource);
    size_t r = 0;
    for (unsigned int i=0; i<nmemb && src->cursor < src->size; i++) {
        size_t a = MathUtil::Min(src->size - src->cursor + 1, (int)size);
        memcpy(&((char*)ptr)[i * size], &src->datas[src->cursor], a);
        src->cursor += a;
        r += a;
    }
    return r;
}

static int seek_func(void* datasource, ogg_int64_t offset, int whence) {
    DataSource* src = static_cast<DataSource*> (datasource);
    switch (whence) {
        case SEEK_SET:
            src->cursor = offset;
            break;
        case SEEK_CUR:
            src->cursor += offset;
            break;
        case SEEK_END:
            src->cursor = src->size - offset;
            break;
    }
    return 0;
}

static long int tell_func(void* datasource) {
    DataSource* src = static_cast<DataSource*> (datasource);
    return src->cursor;
}

static int close_func(void *datasource) {
    DataSource* src = static_cast<DataSource*> (datasource);
    delete src;
    return 0;
}
#endif

#ifdef SAC_INGAME_EDITORS
void MusicSystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif
