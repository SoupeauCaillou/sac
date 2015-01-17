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

#include "OggDecoder.h"
#include "api/AssetAPI.h"
#include "base/Log.h"
#include "base/CircularBuffer.h"

#include <cmath>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#define STB_VORBIS_NO_PUSHDATA_API 1
#include "libs/stb/stb_vorbis.c"
#pragma GCC diagnostic pop

#include <thread>
#include <mutex>
#include <condition_variable>

struct OggHandle {
    stb_vorbis* vb;

    const FileBuffer* fb;
    bool finished;
    bool async;
#if !SAC_WEB
    std::thread decodeThread;
    std::mutex mutex;
    std::condition_variable condition;
#endif
    CircularBuffer<short>* buffer;
};

static void fillInfo(stb_vorbis* vb, OggInfo::Values& info) {
    info.durationSeconds = stb_vorbis_stream_length_in_seconds(vb);
    info.durationSamples = stb_vorbis_stream_length_in_samples(vb);

    stb_vorbis_info i = stb_vorbis_get_info(vb);
    info.sampleRate = i.sample_rate;
    info.numChannels = i.channels;
}

static void initHandle(OggHandle* r) {
    int error = 0;
    r->vb = stb_vorbis_open_memory(r->fb->data, r->fb->size, &error, NULL);
    LOGE_IF(error != VORBIS__no_error, "Error opening ogg file: " << error);

    stb_vorbis_info i = stb_vorbis_get_info(r->vb);

    stb_vorbis_stream_length_in_samples(r->vb);

    r->buffer = new CircularBuffer<short>(i.sample_rate * i.channels * 1); // 1 sec
    r->finished = false;
}

#if !SAC_WEB
static void DecodeThread(OggHandle* hdl, std::mutex* ready, std::condition_variable* cond) {
    ready->lock();

    initHandle(hdl);

    const stb_vorbis_info i = stb_vorbis_get_info(hdl->vb);

    const unsigned chunkSize = 0.1f * i.sample_rate * i.channels;
    short* temp = new short[chunkSize];

    cond->notify_one();
    ready->unlock();

    std::unique_lock<std::mutex> lock(hdl->mutex);

    while (!hdl->finished) {
        if (hdl->buffer->writeSpaceAvailable() >= hdl->buffer->getBufferSize() * 0.5) {
            int count = stb_vorbis_get_samples_short_interleaved(
                hdl->vb, i.channels, temp, chunkSize);
            if (count == 0) {
                LOGV(1, "End of ogg reached. Stop decoding");
                // hdl->finished = true;
                hdl->condition.wait(lock);
            } else {
                hdl->buffer->write(temp, count);
            }
        } else {
            hdl->condition.wait(lock);
        }
    }
}
#endif

OggHandle* OggDecoder::load(const FileBuffer* fb, OggOption::Decoding d) {
    OggHandle* r = new OggHandle;
    r->fb = fb;

    switch (d) {
        case OggOption::Sync:
            LOGV(1, "OggDecoder loading in Sync mode");
            r->async = false;
            initHandle(r);
            break;
#if !SAC_WEB
        case OggOption::Async:
            LOGV(1, "OggDecoder loading in Async mode");
            r->async = true;
            std::mutex m;
            std::condition_variable c;
            std::unique_lock<std::mutex> lock(m);
            r->decodeThread = std::thread(DecodeThread, r, &m, &c);
            c.wait(lock);
            break;
#endif
    }

    return r;
}

const FileBuffer* OggDecoder::release(OggHandle* handle) {
    LOGE_IF(!handle, "Releasing a null handle");
    LOGW_IF(!handle->vb, "OggHandle without valid vorbis?");

#if !SAC_WEB
    if (handle->async) {
        {
            std::unique_lock<std::mutex> lock(handle->mutex);
            handle->finished = true;
            handle->condition.notify_one();
        }
        handle->decodeThread.join();
    }
#endif

    if (handle->vb) {
        stb_vorbis_close(handle->vb);
        handle->vb = 0;
    }
    auto* r = handle->fb;
    delete handle;
    return r;
}

int OggDecoder::availableSamples(OggHandle* handle) {
#if !SAC_WEB
    std::unique_lock<std::mutex> lock(handle->mutex);
    int ret = handle->buffer->readDataAvailable();
    return ret;
#else
    return stb_vorbis_stream_length_in_samples(handle->vb) -
        stb_vorbis_get_sample_offset(handle->vb);
#endif
}

int OggDecoder::readSamples(OggHandle* handle, int numSamples, short* output) {
#if !SAC_WEB
    if (handle->async) {
        std::unique_lock<std::mutex> lock(handle->mutex);
        handle->buffer->read(output, numSamples);
        handle->condition.notify_one();
        return numSamples;
    } else
#endif
    {
        stb_vorbis_info i = stb_vorbis_get_info(handle->vb);
        int count = stb_vorbis_get_samples_short_interleaved(
                handle->vb, i.channels, output, numSamples);
        return count;
    }
}

OggInfo::Values OggDecoder::query(OggHandle* handle) {
    LOGF_IF(!handle->vb, "OggHandle without valid vorbis");
    OggInfo::Values r;
    fillInfo(handle->vb, r);
    return r;
}

int OggDecoder::decode(const FileBuffer& fb, short** output, OggInfo::Values& info) {
    int error = 0;
    stb_vorbis* vb = stb_vorbis_open_memory(fb.data, fb.size, &error, NULL);
    LOGE_IF(error != VORBIS__no_error, "Error opening ogg file: " << error);
    if (!vb)
        return 0;

    fillInfo(vb, info);

    int numShorts = info.durationSamples * info.numChannels;
    *output = new short[numShorts];
    int index = 0;
    while (index < numShorts) {
        int count = stb_vorbis_get_samples_short_interleaved(vb, info.numChannels, &((*output)[index]), numShorts - index);
        if (count == 0)
            break;
        index += count;
    }

    stb_vorbis_close(vb);

    return index;
}
