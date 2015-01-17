#include "FileBufferHelper.h"
#include "api/AssetAPI.h"

const char* FileBufferHelper::line(const FileBuffer& fb, Options opt) {
    uint8_t* cursor = fb.data;
    if (pos >= fb.size)
        return NULL;
    cursor += pos;
    char* nextEndLine = static_cast<char*>(memchr(cursor, '\n', fb.size - pos));

    int len = 0;
    if (nextEndLine) {
        len = nextEndLine - (char*)cursor;
    } else {
        len = fb.size - pos + 1;
    }
    if (opt == Options::AdvanceCursor) {
        pos = pos + len + 1;
    }

    if (allocatedSize < len) {
        allocatedSize = glm::max(len, allocatedSize + 128);
        allocated = (char*) realloc(allocated, allocatedSize);
    }

    memcpy(allocated, cursor, len + 1);
    allocated[len] = '\0';

    return allocated;
}

FileBufferHelper::~FileBufferHelper() {
    if (allocatedSize) free(allocated);
}
