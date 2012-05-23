#include "MusicSystem.h"
#ifdef MUSIC_VISU
#include "RenderingSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"
#endif
#include "base/CircularBuffer.h"

#ifdef ANDROID

#define assert(x) x

#endif

#include <linux/sched.h>

INSTANCE_IMPL(MusicSystem);

MusicSystem::MusicSystem() : ComponentSystemImpl<MusicComponent>("music"), assetAPI(0) { }

static void* _startOggThread(void* arg) {
    static_cast<MusicSystem*>(arg)->oggDecompRunLoop();
    return 0;
}

void MusicSystem::init() {
    musicAPI->init();

    muted = false;
    nextValidRef = 1;

    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&cond, 0);

    pthread_create(&oggDecompressionThread, 0, _startOggThread, this);
    // pthread_setschedprio(oggDecompressionThread, sched_get_priority_min(
    // sched_setscheduler(oggDecompressionThread, SCHED_RR, 0);
}

void MusicSystem::clearAndRemoveInfo(MusicRef ref) {
    std::map<MusicRef, MusicInfo>::iterator it = musics.find(ref);
    if (it == musics.end())
        return;
    LOGW("Delayed erase music ref: %d", ref);
    it->second.toRemove = true;
    pthread_cond_signal(&cond);
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
	clearAndRemoveInfo(m->music);
	clearAndRemoveInfo(m->loopNext);
	m->music = m->loopNext = InvalidMusicRef;
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

        //LOGW("%d -> %p/%p", jt->first, m->opaque[0], m->opaque[1]);
		
        if (m->control == MusicComponent::Start && m->music != InvalidMusicRef && !m->opaque[0]) {
            // start
            m->opaque[0] = startOpaque(m, m->music, m->master, 0);
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
	        if (m->currentVolume != m->volume) {
	            musicAPI->setVolume(m->opaque[0], m->volume);
            }
            // need to queue more data ?
            feed(m->opaque[0], m->music, 0, dt);
            m->positionI = musicAPI->getPosition(m->opaque[0]);
           
           /*	bool desyncFromMaster = (m->master && MathUtil::Abs(1 - m->positionI / (float)m->master->positionI) > 0.05);
            if (desyncFromMaster) {
	            LOGW("Slave track is out of sync : kill it");
            }*/
            assert (m->music != InvalidMusicRef);
            int sampleRate0 = musics[m->music].sampleRate;
            if ((m->music != InvalidMusicRef && m->positionI >= musics[m->music].nbSamples) || !musicAPI->isPlaying(m->opaque[0])) {
                LOGI("%p Player 0 has finished (isPlaying:%d)", m, musicAPI->isPlaying(m->opaque[0]));
                m->positionI = 0;
                musicAPI->deletePlayer(m->opaque[0]);
                m->opaque[0] = 0;
                // remove m->music from musics
                LOGW("Delete m->music: %d", m->music);
                clearAndRemoveInfo(m->music);
                m->music = InvalidMusicRef;
                m->control = MusicComponent::Stop;
            }

			// if [0] is valid, and [1] not, and [0] can loop
            if (m->opaque[0] && !m->opaque[1] && m->loopNext != InvalidMusicRef) {
                bool loop = false;
                if (m->master) {
                    loop = m->master->looped;
                } else {
                    loop = ((m->loopAt > 0) & (m->positionI >= SEC_TO_SAMPLES(m->loopAt, sampleRate0)));
                }

                if (loop) {
                    LOGI("%p Begin loop (%d >= %d) - m->music:%d [master=%p]", m, m->positionI, SEC_TO_SAMPLES(m->loopAt, sampleRate0), m->music, m->master);
                    m->looped = true;
                    m->opaque[1] = m->opaque[0];
                    MusicRef r = m->music;
                    m->music = m->loopNext;
                    m->loopNext = r;
                    
                    if (m->master) {
    	                m->opaque[0] = startOpaque(m, m->music, m->master, 0);
                    } else {
    	                int offset = m->positionI - SEC_TO_SAMPLES(m->loopAt, sampleRate0);
    	                m->opaque[0] = startOpaque(m, m->music, 0, offset);
                    }
                    
                    m->positionI = musicAPI->getPosition(m->opaque[0]);
                }
            }
        } 
        
        if (m->opaque[1]) {
            assert(m->loopNext != InvalidMusicRef);
            if (m->currentVolume != m->volume) {
            	musicAPI->setVolume(m->opaque[1], m->volume);
        	}
            feed(m->opaque[1], m->loopNext, 0, dt);
            if ((m->loopNext != InvalidMusicRef && musicAPI->getPosition(m->opaque[1]) >= musics[m->loopNext].nbSamples) || !musicAPI->isPlaying(m->opaque[1])) {
                musicAPI->deletePlayer(m->opaque[1]);
                m->opaque[1] = 0;
                // remove m->loopNext from musics
                LOGW("Delete m->loopNext:: %d", m->loopNext);
                clearAndRemoveInfo(m->loopNext);
                m->loopNext = InvalidMusicRef;
                LOGI("%p Player 1 has finished", m);
            }
		}

        if (!m->opaque[0] && m->control == MusicComponent::Start && m->master && m->loopNext != InvalidMusicRef) {
	        if (m->master->looped) {
		        LOGI("Restarting because master has looped (current: %d -> next: %d) [%p/%p]", m->music, m->loopNext, m->opaque[0], m->opaque[1]);
		        m->music = m->loopNext;
		        if (m->opaque[1]) {
			        LOGW("Weird, shouldn't happen");
			        musicAPI->deletePlayer(m->opaque[1]);
                	m->opaque[1] = 0;
		        }
                m->loopNext = InvalidMusicRef;
		        m->opaque[0] = startOpaque(m, m->music, m->master, 0);
	        }
        }
        m->currentVolume = m->volume;
        
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

            Entity f = theEntityManager.CreateEntity();
             ADD_COMPONENT(f, Rendering);
             RENDERING(f)->color = Color((size % 2), (size % 2), (size % 2), 0.6);
             RENDERING(f)->hide = false;
             ADD_COMPONENT(f, Transformation);
             TRANSFORM(f)->size = Vector2(0.5, 1);
             TRANSFORM(f)->z = 0.75;

			visualisationEntities[a] = std::make_pair(e, f);
		}
		float VisuWidth = PlacementHelper::GimpWidthToScreen(50);
		Entity e = visualisationEntities[a].first;
        Entity f = visualisationEntities[a].second;
		TRANSFORM(e)->size = Vector2(VisuWidth * 0.5, rc->positionF * PlacementHelper::GimpHeightToScreen(1280));
		TransformationSystem::setPosition(TRANSFORM(e), 
			Vector2(
				PlacementHelper::GimpXToScreen(0) + idx * VisuWidth, PlacementHelper::GimpYToScreen(0)), 
			TransformationSystem::NW);
         if (rc->opaque[1] && rc->loopNext != InvalidMusicRef) {
         float pF = musicAPI->getPosition(rc->opaque[1]) / (float)musics[rc->loopNext].nbSamples;
        TRANSFORM(f)->size = Vector2(VisuWidth * 0.5, pF * PlacementHelper::GimpHeightToScreen(1280));
     TransformationSystem::setPosition(TRANSFORM(f),
         Vector2(
             PlacementHelper::GimpXToScreen(0) + idx * VisuWidth + TRANSFORM(e)->size.X * 0.5, PlacementHelper::GimpYToScreen(0)),
         TransformationSystem::NW);
            RENDERING(f)->hide = false;
         } else {
            RENDERING(f)->hide = true;
         }
		if (rc->control == MusicComponent::Stop) {
			RENDERING(e)->color = RENDERING(f)->color = Color(0.3, 0, 0, 0.5);
		} else if (rc->opaque[0]) {
			RENDERING(e)->color = RENDERING(f)->color = Color(0, 0.3, 0, 0.5);
        } else {
            RENDERING(e)->color = RENDERING(f)->color = Color(0.9, 0.9, 0, 0.5);
        }
		idx++;
	}
	#endif
}

void MusicSystem::oggDecompRunLoop() {
    // who cares
    std::map<MusicRef, std::pair<int8_t*, int> > bigChunks;
    typedef std::map<MusicRef, std::pair<int8_t*, int> >::iterator ChunkIt;
    pthread_mutex_lock(&mutex);

    // one static buffer to rule them all
    int8_t tempBuffer[48000 * 2]; // 1 sec * 48Hz * 2 bytes/sample

    while (true) {
	    bool roomForImprovement = false;
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
    			continue;
            } else {
	            ++it;
            }
            
            int chunkSize = info.pcmBufferSize; //info.buffer->getBufferSize() * 0.5;
            
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
        // release mutex while waiting
        if (!roomForImprovement) {
        	pthread_cond_wait(&cond, &mutex);
        	// mutex is auto acquired on wake up
        }
    }
}

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
	    unsigned int count = info.buffer->readDataAvailable();
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

OpaqueMusicPtr* MusicSystem::startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset) {
    assert (r != InvalidMusicRef);
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

	musicAPI->startPlaying(ptr, master ? master->opaque[0] : 0, offset);
	// set volume
	musicAPI->setVolume(ptr, m->volume);
	m->currentVolume = m->volume;
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

static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource);
static int seek_func(void* datasource, ogg_int64_t offset, int whence);
static long int tell_func(void* datasource);
static int close_func(void *datasource);

struct DataSource {
    uint8_t* datas;
    int size;
    int cursor;
};

MusicRef MusicSystem::loadMusicFile(const std::string& assetName) {
	if (!assetAPI)
		return InvalidMusicRef;
		
    FileBuffer b;
    int size;
    if (name2buffer.find(assetName) == name2buffer.end()) {
        b = assetAPI->loadAsset(assetName);
        name2buffer[assetName] = b;
    } else {
        b = name2buffer[assetName];
    }

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
    LOGI("File: %s / rate: %d duration: %.3f nbSample: %d -> %d", assetName.c_str(), info.sampleRate, info.totalTime, info.nbSamples, nextValidRef);
    pthread_mutex_lock(&mutex);
    musics[nextValidRef] = info;
    pthread_mutex_unlock(&mutex);
    // wake-up decompression thread
    pthread_cond_signal(&cond);

    return nextValidRef++;
}

int MusicSystem::decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize) {
    int bitstream;
    int read = 0;
    while (read < chunkSize) {
        int n = ov_read(file, (char*) &data[read], chunkSize - read, &bitstream);
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

static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    DataSource* src = static_cast<DataSource*> (datasource);
    size_t r = 0;
    for (int i=0; i<nmemb && src->cursor < src->size; i++) {
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
    LOGW("close_func : %p", datasource);
    DataSource* src = static_cast<DataSource*> (datasource);
    delete src;
    return 0;
}