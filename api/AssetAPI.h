#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include <cstring>
#include "base/MathUtil.h"

struct FileBuffer {
    FileBuffer() : data(0), size(0) {}
    uint8_t* data;
    int size;
};

class AssetAPI {
    public:
        virtual FileBuffer loadAsset(const std::string& asset) = 0;
        virtual std::list<std::string> listContent(const std::string& extension, const std::string& subfolder = "") = 0;
};


struct FileBufferWithCursor : FileBuffer {
    FileBufferWithCursor(const FileBuffer& fb) : cursor(0) {
        data = fb.data;
        size = fb.size;
    }
    FileBufferWithCursor() : FileBuffer(), cursor(0) {}
    long int cursor;

    static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
        FileBufferWithCursor* src = static_cast<FileBufferWithCursor*> (datasource);
        size_t r = 0;
        for (unsigned int i=0; i<nmemb && src->cursor < src->size; i++) {
            size_t a = MathUtil::Min(src->size - src->cursor + 1, (long int)size);
            memcpy(&((char*)ptr)[i * size], &src->data[src->cursor], a);
            src->cursor += a;
            r += a;
        }
        return r;
    }

    static int seek_func(void* datasource, int64_t offset, int whence) {
        FileBufferWithCursor* src = static_cast<FileBufferWithCursor*> (datasource);
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
        FileBufferWithCursor* src = static_cast<FileBufferWithCursor*> (datasource);
        return src->cursor;
    }

    static int close_func(void *datasource) {
        FileBufferWithCursor* src = static_cast<FileBufferWithCursor*> (datasource);
        delete src;
        return 0;
    }
};
