#include "MusicSystem.h"

#define SEC_TO_BYTE(s, freq) (int)(s * freq * 2)
#define MUSIC_CHUNK_SIZE(freq) SEC_TO_BYTE(0.5, freq)

INSTANCE_IMPL(MusicSystem);

MusicSystem::MusicSystem() : ComponentSystemImpl<MusicComponent>("music") { }

void MusicSystem::init() {
    musicAPI->init();
}

void MusicSystem::DoUpdate(float dt) {
    for(std::map<Entity, MusicComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
        MusicComponent* m = (*jt).second;

        if (m->control == MusicComponent::Start && m->music != InvalidMusicRef && !m->opaque[0]) {
            // start
            m->opaque[0] = startOpaque(m->music);
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
            feed(m->opaque[0], m->music);
            m->position = musicAPI->getPosition(m->opaque[0]);
            if (!musicAPI->isPlaying(m->opaque[0])) {
                LOGI("Player 0 has finished");
                m->position = 0;
                musicAPI->deletePlayer(m->opaque[0]);
                m->opaque[0] = 0;
                m->music = InvalidMusicRef;
            }

            int sampleRate0 = musics[m->music].sampleRate;
            if (m->opaque[1]) {
                feed(m->opaque[1], m->loopNext);
                if (!musicAPI->isPlaying(m->opaque[1])) {
                    musicAPI->deletePlayer(m->opaque[1]);
                    m->opaque[1] = 0;
                    m->loopNext = InvalidMusicRef;
                    LOGI("Player 1 has finished");
                }
            } else if (m->loopAt > 0 && m->position >= SEC_TO_BYTE(m->loopAt, sampleRate0)) {
                LOGI("Begin loop");
                m->opaque[1] = m->opaque[0];
                MusicRef r = m->music;
                m->music = m->loopNext;
                m->loopNext = r;
                m->opaque[0] = startOpaque(m->music);
                int offset = m->position - SEC_TO_BYTE(m->loopAt, sampleRate0);
                // queue necessary data
                int amount = (int) (offset / MUSIC_CHUNK_SIZE(sampleRate0)) + 1;
                for (int i=1; i<amount; i++) {
                    feed(m->opaque[0], m->music);
                }
                musicAPI->setPosition(m->opaque[0], offset);
            }
        }
    }
}

bool MusicSystem::feed(OpaqueMusicPtr* ptr, MusicRef m) {
    MusicInfo info = musics[m];

    while (musicAPI->needData(ptr, info.sampleRate)) {
        int8_t* data = 0;
        int size = decompressNextChunk(info.ovf, &data, MUSIC_CHUNK_SIZE(info.sampleRate));
        if (size == 0) { // EOF
            return false;
        }
        musicAPI->queueMusicData(ptr, data, size, info.sampleRate);
    }
    return true;
}

OpaqueMusicPtr* MusicSystem::startOpaque(MusicRef r) {
    OpaqueMusicPtr* ptr = musicAPI->createPlayer();
    MusicInfo info = musics[r];
    int8_t* data = 0;
    int size = decompressNextChunk(musics[r].ovf, &data, MUSIC_CHUNK_SIZE(info.sampleRate));
    musicAPI->queueMusicData(ptr, data, size, info.sampleRate);
    musicAPI->startPlaying(ptr);
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