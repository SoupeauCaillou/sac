#include "MusicSystem.h"
#ifdef MUSIC_VISU
#include "RenderingSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"
#endif

#ifdef ANDROID

#define assert(x) x

#endif

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
}

void MusicSystem::clearAndRemoveInfo(MusicRef ref) {
    std::map<MusicRef, MusicInfo>::iterator it = musics.find(ref);
    if (it == musics.end())
        return;
    LOGW("Erase music ref: %d", ref);
    pthread_mutex_lock(&mutex);
    if (it->second.ovf)
        ov_clear(it->second.ovf);
    // deallocate nextPcmBuffer to
    musics.erase(it);
    pthread_mutex_unlock(&mutex);
}

void MusicSystem::stopMusic(MusicComponent* m) {
    for (int i=0; i<2; i++) {
        if (m->opaque[i]) {
            musicAPI->stopPlayer(m->opaque[i]);
            musicAPI->deletePlayer(m->opaque[i]);
            m->opaque[i] = 0;
            m->positionI = m->positionF = 0;
            clearAndRemoveInfo(m->music);
            clearAndRemoveInfo(m->loopNext);
            m->music = m->loopNext = InvalidMusicRef;
        }
    }
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
		
        if (m->control == MusicComponent::Start && m->music != InvalidMusicRef && !m->opaque[0]) {
            // start
            m->opaque[0] = startOpaque(m, m->music, m->master, 0);
            m->opaque[1] = 0;
        } else if (m->control == MusicComponent::Stop && m->opaque[0]) {
            stopMusic(m);
        }

        // playing
        if (m->opaque[0]) {
            // need to queue more data ?
            feed(m->opaque[0], m->music, 0);
            m->positionI = musicAPI->getPosition(m->opaque[0]);
            assert (m->music != InvalidMusicRef);
            int sampleRate0 = musics[m->music].sampleRate;
            if ((m->music != InvalidMusicRef && m->positionI >= musics[m->music].nbSamples) || !musicAPI->isPlaying(m->opaque[0])) {
                LOGI("%p Player 0 has finished", m);
                m->positionI = 0;
                musicAPI->deletePlayer(m->opaque[0]);
                m->opaque[0] = 0;
                // remove m->music from musics
                clearAndRemoveInfo(m->music);
                m->music = InvalidMusicRef;
                m->control = MusicComponent::Stop;
            }

            if (m->opaque[1]) {
                assert(m->loopNext != InvalidMusicRef);
                feed(m->opaque[1], m->loopNext, 0);
                if ((m->loopNext != InvalidMusicRef && musicAPI->getPosition(m->opaque[1]) >= musics[m->loopNext].nbSamples) || !musicAPI->isPlaying(m->opaque[1])) {
                    musicAPI->deletePlayer(m->opaque[1]);
                    m->opaque[1] = 0;
                    // remove m->loopNext from musics
                    clearAndRemoveInfo(m->loopNext);
                    m->loopNext = InvalidMusicRef;
                    LOGI("%p Player 1 has finished", m);
                }
                else {
	                /*int p1 = musicAPI->getPosition(m->opaque[0]);
	                int p2 = musicAPI->getPosition(m->opaque[1]);
                	LOGI("%d __ %d : %.3f", p1, p2, SAMPLES_TO_SEC(p2 - p1, sampleRate0));*/
                }
            }
            else if (m->loopNext != InvalidMusicRef) {
                bool loop = false;
                if (m->master) {
                    loop = m->master->looped;
                } else {
                    loop = ((m->loopAt > 0) & (m->positionI >= SEC_TO_SAMPLES(m->loopAt, sampleRate0)));
                }

                if (loop) {
                    LOGI("%p Begin loop (%d >= %d)", m, m->positionI, SEC_TO_SAMPLES(m->loopAt, sampleRate0));
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
        } else if (m->control == MusicComponent::Start && m->master) {
	        if (m->master->looped) {
		        LOGI("Restarting because master has looped (current: %d -> next: %d)", m->music, m->loopNext);
		        m->music = m->loopNext;
                m->loopNext = InvalidMusicRef;
		        m->opaque[0] = startOpaque(m, m->music, m->master, 0);
	        }
        }
        
        if (m->music != InvalidMusicRef) {
        	m->positionF = m->positionI / (float)musics[m->music].nbSamples;
        }
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
			TRANSFORM(e)->size = Vector2(1, 1);
			TRANSFORM(e)->z = 0.75;
			visualisationEntities[a] = e;
		}
		float VisuWidth = PlacementHelper::GimpWidthToScreen(50);
		Entity e = visualisationEntities[a];
		TRANSFORM(e)->size = Vector2(VisuWidth, rc->positionF * PlacementHelper::GimpHeightToScreen(1280));
		TransformationSystem::setPosition(TRANSFORM(e), 
			Vector2(
				PlacementHelper::GimpXToScreen(0) + idx * VisuWidth, PlacementHelper::GimpYToScreen(0)), 
			TransformationSystem::NW);
		if (rc->control == MusicComponent::Stop) {
			RENDERING(e)->color = Color(0.3, 0, 0, 0.5);
		} else if (rc->opaque[0]) {
			RENDERING(e)->color = Color(0, 0.3, 0, 0.5);
        } else {
            RENDERING(e)->color = Color(0.9, 0.9, 0, 0.5);
        }
		idx++;
	}
	#endif
}

void MusicSystem::oggDecompRunLoop() {
    pthread_mutex_lock(&mutex);

    while (true) {
        // LOGW("OGG DECOMP LOOP STARTED");
        for (std::map<MusicRef, MusicInfo>::iterator it=musics.begin(); it!=musics.end(); ++it) {
            MusicInfo& info = it->second;

            assert(it->first != InvalidMusicRef);
            if (info.nextPcmBuffer != 0 && info.newBufferRequested) {
                // LOGW("decode 1 buffer : %d -> %p", it->first, info.ovf);
                decompressNextChunk(info.ovf, info.nextPcmBuffer, info.pcmBufferSize);
                info.newBufferRequested = false;
            }
        }
        // release mutex while waiting
        //  LOGW("OGG DECOMP WAIT");
        pthread_cond_wait(&cond, &mutex);
        // mutex is auto acquired
    }
}

bool MusicSystem::feed(OpaqueMusicPtr* ptr, MusicRef m, int forceFeedCount) {
    assert (m != InvalidMusicRef);
    MusicInfo& info = musics[m];
    int count = musicAPI->needData(ptr, info.sampleRate, forceFeedCount > 0);
    if (!count)
        return true;

    pthread_mutex_lock(&mutex);
    if (!info.nextPcmBuffer) {
        LOGW("No audio data available - crap");
    } else {
        // LOGW("using buffer: %p", info.nextPcmBuffer);
        musicAPI->queueMusicData(ptr, info.nextPcmBuffer, info.pcmBufferSize, info.sampleRate);
        info.nextPcmBuffer = musicAPI->allocate(info.pcmBufferSize);
        info.newBufferRequested = true;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);

    return true;
}

OpaqueMusicPtr* MusicSystem::startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset) {
    assert (r != InvalidMusicRef);
	MusicInfo info = musics[r];
    if (info.sampleRate <=0) {
        LOGW("Invalid sample rate: %d", info.sampleRate);
    }
    OpaqueMusicPtr* ptr = m->opaque[0] = musicAPI->createPlayer(info.sampleRate);

    // init with 2 silence pkt
    int8_t* buffer0 = musicAPI->allocate(info.pcmBufferSize);
    memset(buffer0, 0,  info.pcmBufferSize);
    musicAPI->queueMusicData(ptr, buffer0, info.pcmBufferSize, info.sampleRate);

    int8_t* buffer1 = musicAPI->allocate(info.pcmBufferSize);
    memset(buffer1, 0,  info.pcmBufferSize);
    musicAPI->queueMusicData(ptr, buffer1, info.pcmBufferSize, info.sampleRate);

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
    LOGI("todo");
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
	LOGW("assAPI : %p", assetAPI);
	if (!assetAPI)
		return InvalidMusicRef;
		
    FileBuffer b;
    int size;
    if (name2buffer.find(assetName) == name2buffer.end()) {
        name2buffer[assetName] = b = assetAPI->loadAsset(assetName);
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
    info.sampleRate = inf->rate * inf->channels;
    info.nbSamples = ov_pcm_total(f, -1);
    info.pcmBufferSize = musicAPI->pcmBufferSize(info.sampleRate);
    info.nextPcmBuffer = musicAPI->allocate(info.pcmBufferSize);
    info.newBufferRequested = true;
    LOGI("File: %s / rate: %d duration: %.3f nbSample: %d -> %d", assetName.c_str(), info.sampleRate, info.totalTime, info.nbSamples, nextValidRef);
    pthread_mutex_lock(&mutex);
    musics[nextValidRef] = info;
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cond);

    return nextValidRef++;
}

int MusicSystem::decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize) {
    int bitstream;
    int read = 0;
    while (read < chunkSize) {
        int n = ov_read(file, (char*) &data[read], chunkSize - read, &bitstream);
        if (n == 0) {
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
        LOGI("Producing %d/%d bytes of silence", chunkSize - read - 1, chunkSize);
        memset(&data[read], 0, chunkSize - read - 1);
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