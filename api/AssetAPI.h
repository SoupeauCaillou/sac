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



#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include <cstring>
#include <glm/glm.hpp>

struct FileBuffer {
    FileBuffer() : data(0), size(0) {}
    uint8_t* data;
    int size;
};

class AssetAPI {
    public:
        //open file at path 'fullpath'
        virtual FileBuffer loadFile(const std::string& fullpath) = 0;
        //open a file bundled with the game, in assets/ or assetspc/
        virtual FileBuffer loadAsset(const std::string& asset) = 0;

        //get the list of filenames in directory containing "extension" in their name
        virtual std::list<std::string> listContent(const std::string& directory, const std::string& extension, const std::string& subfolder = "") = 0;
        //get the list of filenames in assets directory containing "extension" in their name
        virtual std::list<std::string> listAssetContent(const std::string& extension, const std::string& subfolder = "") = 0;

        virtual void createDirectory(const std::string& fullpath) = 0;
        virtual bool doesExistFileOrDirectory(const std::string& fullpath) = 0;
        virtual void removeFileOrDirectory(const std::string& fullpath) = 0;

        //get the writable directory for the game, it is platform dependent
        virtual const std::string & getWritableAppDatasPath() = 0;
        //write file in filesystem (emscripten only)
        virtual void synchronize() = 0;
};


struct FileBufferWithCursor : FileBuffer {
    FileBufferWithCursor(const FileBuffer& fb, bool pDeleteThisOnClose = false) : cursor(0) {
        data = fb.data;
        size = fb.size;
        deleteThisOnClose = pDeleteThisOnClose;
    }
    FileBufferWithCursor() : FileBuffer(), cursor(0), deleteThisOnClose(false) {}
    long int cursor;
    bool deleteThisOnClose;

    static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
        FileBufferWithCursor* src = static_cast<FileBufferWithCursor*> (datasource);
        size_t r = 0;
        for (unsigned int i=0; i<nmemb && src->cursor < src->size; i++) {
            size_t a = glm::min(src->size - src->cursor + 1, (long int)size);
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
        if (src->deleteThisOnClose) delete src;
        return 0;
    }
};
