#include "MusicSystem.h"

// 500 ms 8000Hz 16b/s
#define MUSIC_CHUNK_SIZE 8000

INSTANCE_IMPL(MusicSystem);

MusicSystem::MusicSystem() : ComponentSystemImpl<MusicComponent>("music") { }

void MusicSystem::init() {
    musicAPI->init();
}

void MusicSystem::DoUpdate(float dt) {
    for(std::map<Entity, MusicComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
        MusicComponent* m = (*jt).second;

        if (m->control == MusicComponent::Start && m->music != InvalidMusicRef && !m->opaque) {
            // start
            m->opaque = musicAPI->createPlayer();
            int8_t* data = 0;
            int size = decompressNextChunk(musics[m->music], &data);
            musicAPI->queueMusicData(m->opaque, data, size);
            musicAPI->startPlaying(m->opaque);
        } else if (m->control == MusicComponent::Stop && m->opaque) {
            // stop
            musicAPI->stopPlayer(m->opaque);
            musicAPI->deletePlayer(m->opaque);
            m->opaque = 0;
        }

        // playing
        if (m->opaque) {
            // need to queue more data ?
            while (musicAPI->needData(m->opaque)) {
                int8_t* data = 0;
                int size = decompressNextChunk(musics[m->music], &data);
                if (size == 0) { // EOF
                    break;
                }
                musicAPI->queueMusicData(m->opaque, data, size);
            }
        }
    }
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

    musics[nextValidRef] = f;
    return nextValidRef++;
}

int MusicSystem::decompressNextChunk(OggVorbis_File* file, int8_t** data) {
    *data = new int8_t[MUSIC_CHUNK_SIZE];
    int bitstream;
    int read = 0;
    while (read < MUSIC_CHUNK_SIZE) {
        int n = ov_read(file, (char*) &(*data)[read], MUSIC_CHUNK_SIZE - read, &bitstream);
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