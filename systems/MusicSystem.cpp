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

void MusicSystem::init() {
    musicAPI->init();
}

void MusicSystem::DoUpdate(float dt) {
    for(std::map<Entity, MusicComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
        MusicComponent* m = (*jt).second;

		m->looped = false;
		
        if (m->control == MusicComponent::Start && m->music != InvalidMusicRef && !m->opaque[0]) {
            // start
            m->opaque[0] = startOpaque(m, m->music, m->master, 0);
            m->opaque[1] = 0;
        } else if (m->control == MusicComponent::Stop && m->opaque[0]) {
            // stop
            for (int i=0; i<2; i++) {
                if (m->opaque[i]) {
                    musicAPI->stopPlayer(m->opaque[i]);
                    musicAPI->deletePlayer(m->opaque[i]);
                    m->opaque[i] = 0;
                }
            }
        }

        // playing
        if (m->opaque[0]) {
            // need to queue more data ?
            feed(m->opaque[0], m->music, 0);
            m->positionI = musicAPI->getPosition(m->opaque[0]);
            if (!musicAPI->isPlaying(m->opaque[0])) {
                LOGI("%p Player 0 has finished", m);
                m->positionI = 0;
                musicAPI->deletePlayer(m->opaque[0]);
                m->opaque[0] = 0;
                m->music = InvalidMusicRef;
                m->control = MusicComponent::Stop;
            }

            int sampleRate0 = musics[m->music].sampleRate;
            if (m->opaque[1]) {
                feed(m->opaque[1], m->loopNext, 0);
                if (!musicAPI->isPlaying(m->opaque[1])) {
                    musicAPI->deletePlayer(m->opaque[1]);
                    m->opaque[1] = 0;
                    m->loopNext = InvalidMusicRef;
                    LOGI("%p Player 1 has finished", m);
                }
                else {
	                /*int p1 = musicAPI->getPosition(m->opaque[0]);
	                int p2 = musicAPI->getPosition(m->opaque[1]);
                	LOGI("%d __ %d : %.3f", p1, p2, SAMPLES_TO_SEC(p2 - p1, sampleRate0));*/
                }
            } else if (m->loopNext != InvalidMusicRef && m->loopAt > 0 && m->positionI >= SEC_TO_SAMPLES(m->loopAt, sampleRate0)) {
                LOGI("%p Begin loop", m);
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
        } else if (m->control == MusicComponent::Start && m->master) {
	        if (m->master->looped) {
		        LOGI("Restarting because master has looped");
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
		if (rc->control == MusicComponent::Stop)
			RENDERING(e)->color = Color(0.3, 0, 0, 0.5);
		else
			RENDERING(e)->color = Color(0, 0.3, 0, 0.5);
		idx++;
	}
	#endif
}

bool MusicSystem::feed(OpaqueMusicPtr* ptr, MusicRef m, int forceFeedCount) {
    MusicInfo info = musics[m];

    while (forceFeedCount > 0 || musicAPI->needData(ptr, info.sampleRate)) {
        int8_t* data = 0;
        int size = decompressNextChunk(info.ovf, &data, MUSIC_CHUNK_SIZE(info.sampleRate));
        if (size == 0) { // EOF
        	// LOGW("Feed failed");
            return false;
        }
        musicAPI->queueMusicData(ptr, data, size, info.sampleRate);
        forceFeedCount--;
    }
    return true;
}

OpaqueMusicPtr* MusicSystem::startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset) {
	MusicInfo info = musics[r];
    OpaqueMusicPtr* ptr = m->opaque[0] = musicAPI->createPlayer(info.sampleRate);
        
    if (master) {
	   offset += master->positionI; 
    }
   	// queue necessary data
    int amount = (int) (SAMPLES_TO_BYTE(offset, info.sampleRate) / MUSIC_CHUNK_SIZE(info.sampleRate)) + 1;
    LOGI("Start: %d queued", amount);
    for (int i=0; i<amount; i++) {
    	assert (feed(m->opaque[0], r, 1));
	}
	
	musicAPI->startPlaying(ptr, master ? master->opaque[0] : 0, offset);
                
    return ptr;
}

void MusicSystem::toggleMute(bool enable) {
    LOGI("todo");
}

static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource);
static int seek_func(void* datasource, ogg_int64_t offset, int whence);
static long int tell_func(void* datasource);

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
    cb.close_func = 0;
    cb.tell_func = &tell_func;

    OggVorbis_File* f = new OggVorbis_File();
    ov_open_callbacks(dataSource, f, 0, 0, cb);

    MusicInfo info;
    info.ovf = f;
    info.totalTime = ov_time_total(f, -1) * 0.001;
    vorbis_info* inf = ov_info(f, -1);
    info.sampleRate = inf->rate * inf->channels;
    info.nbSamples = ov_pcm_total(f, -1);
    LOGI("File: %s / rate: %d duration: %.3f nbSample: %d", assetName.c_str(), info.sampleRate, info.totalTime, info.nbSamples);
    musics[nextValidRef] = info;
    return nextValidRef++;
}

int MusicSystem::decompressNextChunk(OggVorbis_File* file, int8_t** data, int chunkSize) {
    *data = new int8_t[chunkSize];
    int bitstream;
    int read = 0;
    while (read < chunkSize) {
        int n = ov_read(file, (char*) &(*data)[read], chunkSize - read, &bitstream);
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
    return read;
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