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
#include "MusicSystem.h"
#ifdef MUSIC_VISU
#include "TextRenderingSystem.h"
#include "RenderingSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"
#endif
#include "base/CircularBuffer.h"

#ifdef ANDROID
#define assert(x) x
#endif

#ifndef EMSCRIPTEN
#ifdef ANDROID
#include "tremor/ivorbisfile.h"
#else
#include <vorbis/vorbisfile.h>
#endif
#include <linux/sched.h>
#else
#include <SDL/SDL_mixer.h>
#include <sstream>
#endif



INSTANCE_IMPL(MusicSystem);

MusicSystem::MusicSystem() : ComponentSystemImpl<MusicComponent>("music"), assetAPI(0) { }

#ifndef EMSCRIPTEN
static void* _startOggThread(void* arg) {
    static_cast<MusicSystem*>(arg)->oggDecompRunLoop();
    return 0;
}
#endif

void MusicSystem::init() {
    muted = false;
    nextValidRef = 1;

#ifndef EMSCRIPTEN
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);

    pthread_create(&oggDecompressionThread, 0, _startOggThread, this);
    // pthread_setschedprio(oggDecompressionThread, sched_get_priority_min(
    // sched_setscheduler(oggDecompressionThread, SCHED_RR, 0);
#endif
}

void MusicSystem::uninit() {
#ifndef EMSCRIPTEN
    runDecompLoop = false;
     pthread_cond_signal(&cond);
    pthread_join(oggDecompressionThread, 0);
    LOGW("MusicSystem uninitinalized");
  #endif
}

void MusicSystem::clearAndRemoveInfo(MusicRef ref) {
#ifndef EMSCRIPTEN
	if (ref == InvalidMusicRef)
		return;
	pthread_mutex_lock(&mutex);
    std::map<MusicRef, MusicInfo>::iterator it = musics.find(ref);
    if (it == musics.end()) {
	    LOGW("Weird, cannot find: %d music ref", ref);
    } else {
    	// LOGW("Delayed erase music ref: %d", ref);
    	it->second.toRemove = true;
    }
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cond);
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
            #ifdef MUSIC_VISU
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
         for(std::map<Entity, MusicComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
            MusicComponent* m = (*jt).second;
            stopMusic(m);
        }
        return;
    }
    
    for(std::map<Entity, MusicComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
        MusicComponent* m = (*jt).second;

		m->looped = false;

		// Music is not started and is startable => launch opaque[0] player		
        if (m->control == MusicComponent::Start && m->music != InvalidMusicRef && !m->opaque[0]) {
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
        } else if (m->control == MusicComponent::Stop && m->opaque[0]) {
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
        }

        // playing
        if (m->opaque[0]) {
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
            #ifndef EMSCRIPTEN
            feed(m->opaque[0], m->music, 0, dt);
            #endif
            m->positionI = musicAPI->getPosition(m->opaque[0]);

            assert (m->music != InvalidMusicRef);
            #ifndef EMSCRIPTEN
            int sampleRate0 = musics[m->music].sampleRate;
            if ((m->music != InvalidMusicRef && m->positionI >= musics[m->music].nbSamples) || !musicAPI->isPlaying(m->opaque[0])) {
	        #else
	        if (!musicAPI->isPlaying(m->opaque[0])) {
	        #endif
	            LOGI("(music) %p Player 0 has finished (isPlaying:%d pos:%d m->music:%d)", m, musicAPI->isPlaying(m->opaque[0]), m->positionI, m->music);
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
	                #ifndef EMSCRIPTEN
                    loop = ((m->loopAt > 0) & (m->positionI >= SEC_TO_SAMPLES(m->loopAt, sampleRate0)));
                    #else
                    loop = ((m->loopAt > 0) & !musicAPI->isPlaying(m->opaque[0]));
                    #endif
                }

                if (loop) {
	                #ifndef EMSCRIPTEN
                    LOGI("(music) %p Begin loop (%d >= %d) - m->music:%d becomas loopNext:%d [master=%p]", m, m->positionI, SEC_TO_SAMPLES(m->loopAt, sampleRate0), m->music, m->loopNext, m->master);
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
	                    #ifndef EMSCRIPTEN
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
            assert(m->previousEnding != InvalidMusicRef);
            if (m->currentVolume != m->volume) {
            	musicAPI->setVolume(m->opaque[1], m->volume);
        	}
        	#ifndef EMSCRIPTEN
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
                LOGI("(music) %p Player 1 has finished", m);
            }
		}

        if (!m->opaque[0] && m->control == MusicComponent::Start && m->master && m->loopNext != InvalidMusicRef) {
	        if (m->master->looped) {
		        LOGI("(music) Restarting because master has looped (current: %d -> next: %d) [%p/%p]", m->music, m->loopNext, m->opaque[0], m->opaque[1]);
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
        
        #ifdef MUSIC_VISU
        if (m->music != InvalidMusicRef) {
        	m->positionF = m->positionI / (float)musics[m->music].nbSamples;
        }
        #endif
    }
    
	#ifdef MUSIC_VISU
	int idx = 0;
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		MusicComponent* rc = (*it).second;
		
		if (visualisationEntities.find(a) == visualisationEntities.end()) {
			int size = visualisationEntities.size();

			Entity e = theEntityManager.CreateEntity();
			ADD_COMPONENT(e, Rendering);
			RENDERING(e)->color = Color((size % 2), (size % 2), (size % 2), 0.6);
			RENDERING(e)->hide = false;
			ADD_COMPONENT(e, Transformation);
			TRANSFORM(e)->size = Vector2(0.5, 1);
			TRANSFORM(e)->z = 0.75;
			ADD_COMPONENT(e, TextRendering);
			TEXT_RENDERING(e)->charHeight = 0.4;
			TEXT_RENDERING(e)->color = Color(0,0,0);
			TEXT_RENDERING(e)->text = "A";
			TEXT_RENDERING(e)->hide = false;

            Entity f = theEntityManager.CreateEntity();
             ADD_COMPONENT(f, Rendering);
             RENDERING(f)->color = Color((size % 2), (size % 2), (size % 2), 0.6);
             RENDERING(f)->hide = false;
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
	        RENDERING(f)->hide = rc->opaque[1] ? false : true;
         } else {
			RENDERING(f)->hide = true;
         }
         
         if (rc->music != InvalidMusicRef) {
	         if (rc->loopNext != InvalidMusicRef) {
		        TEXT_RENDERING(e)->text = musics[rc->music].name + "}" + musics[rc->loopNext].name;
	         } else {
	         	TEXT_RENDERING(e)->text = musics[rc->music].name;
	         }
	         TEXT_RENDERING(e)->hide = false;
         } else if (rc->loopNext != InvalidMusicRef) {
	         TEXT_RENDERING(e)->text = "}" + musics[rc->loopNext].name;
	         TEXT_RENDERING(e)->hide = false;
         } else {
	         TEXT_RENDERING(e)->hide = true;
         }
         if (rc->previousEnding != InvalidMusicRef) {
	         TEXT_RENDERING(f)->text = musics[rc->previousEnding].name;
	         TEXT_RENDERING(f)->hide = false;
         } else {
	         TEXT_RENDERING(f)->hide = true;
         }
         
		if (rc->control == MusicComponent::Stop) {
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

#ifndef EMSCRIPTEN
void MusicSystem::oggDecompRunLoop() {
    runDecompLoop = true;

    pthread_mutex_lock(&mutex);

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
	    			LOGW("delete %p", info.buffer);
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
        	pthread_cond_wait(&cond, &mutex);
        	// mutex is auto acquired on wake up
        }
    }
}
#endif

#ifndef EMSCRIPTEN
bool MusicSystem::feed(OpaqueMusicPtr* ptr, MusicRef m, int forceFeedCount, float dt) {
    assert (m != InvalidMusicRef);
    if (musics.find(m) == musics.end()) {
	    LOGW("Achtung, musicref : %d not found", m);
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
		    pthread_cond_signal(&cond);
	    }
	    if (count >= info.pcmBufferSize) {
		    int8_t* b = musicAPI->allocate(info.pcmBufferSize);
		    info.buffer->read(b, info.pcmBufferSize);
		    musicAPI->queueMusicData(ptr, b, info.pcmBufferSize, info.sampleRate);
		    dt -= chunkDuration;
	    } else {
		    LOGW("Fcuk, not enough data: %u < %d", count, info.pcmBufferSize);
		    break;
	    }
	}
	info.leftOver = dt;

    return true;
}
#endif

OpaqueMusicPtr* MusicSystem::startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset) {
    assert (r != InvalidMusicRef);
#ifndef EMSCRIPTEN
	MusicInfo& info = musics[r];
    if (info.sampleRate <=0) {
        LOGW("Invalid sample rate: %d", info.sampleRate);
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
		LOGW("(music) volume - Start with fading: %.2f - %.2f", m->fadeIn, m->volume);
	} else {
		LOGW("(music) volume - Start without fading: %.2f - %.2f", m->fadeIn, m->volume);
		musicAPI->setVolume(ptr, m->volume);
		m->currentVolume = m->volume;
	}
	//musicAPI->setVolume(ptr, m->volume);
	musicAPI->startPlaying(ptr, master ? master->opaque[0] : 0, offset);
    return ptr;
}

void MusicSystem::toggleMute(bool enable) {
    if (enable && !muted) {
        muted = true;
        for(std::map<Entity, MusicComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
            MusicComponent* m = (*jt).second;
            stopMusic(m);
        }
    } else if (!enable && muted) {
        muted = false;
    }
}

#ifndef EMSCRIPTEN
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
	LOGI("loadMusicFile %s", assetName.c_str());
	if (!assetAPI)
		return InvalidMusicRef;
	PROFILE("Music", "loadMusicFile", BeginEvent);
	#ifndef EMSCRIPTEN
    FileBuffer b;
    if (name2buffer.find(assetName) == name2buffer.end()) {
        b = assetAPI->loadAsset(assetName);
        if (!b.data) {
	        LOGE("Unable to load %s", assetName.c_str());
	        PROFILE("Music", "loadMusicFile", EndEvent);
	        return InvalidMusicRef;
        }
        name2buffer[assetName] = b;
    } else {
	    b = name2buffer[assetName];
    }
    #endif

#ifndef EMSCRIPTEN
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
    ov_open_callbacks(dataSource, f, 0, 0, cb);

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
    LOGI("(music) File: %s / rate: %d duration: %.3f nbSample: %d -> %d", assetName.c_str(), info.sampleRate, info.totalTime, info.nbSamples, nextValidRef);
    pthread_mutex_lock(&mutex);
    #ifdef MUSIC_VISU
    int start = assetName.find("audio/") + 6;
    int end = assetName.find(".ogg");
    info.name = assetName.substr(start, end - start);
    #endif
    musics[nextValidRef] = info;
    // LOGI("================================ ++ %d => %lu", nextValidRef, musics.size());
    pthread_mutex_unlock(&mutex);
    // wake-up decompression thread
    pthread_cond_signal(&cond);
#else
	std::stringstream a;
	a << "assets/" << assetName;
    musics[nextValidRef] = Mix_LoadWAV(a.str().c_str());    
    LOGI("Load music file %s, result: %p", a.str().c_str(), musics[nextValidRef]); 
#endif
	PROFILE("Music", "loadMusicFile", EndEvent);
    return nextValidRef++;
}

#ifndef EMSCRIPTEN
int MusicSystem::decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize) {
    int bitstream;
    int read = 0;
    while (read < chunkSize) {
	    #ifdef ANDROID
        int n = ov_read(file, (char*) &data[read], chunkSize - read, &bitstream);
        #else
        int n = ov_read(file, (char*) &data[read], chunkSize - read, 0, 2, 1, &bitstream);
        #endif
        if (n == 0) {
	        // LOGI("%p] EOF (read: %d/%d)", file, read, chunkSize);
            // EOF
            break;
        } else if (n < 0) {
            LOGI("Error in vorbis read: %d", n);
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

#ifndef EMSCRIPTEN
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