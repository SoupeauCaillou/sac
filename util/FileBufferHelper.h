
struct FileBuffer;

class FileBufferHelper {
public:
    enum Options {
        AdvanceCursor,
        HoldCursor
    };


    const char* line(const FileBuffer& fb, Options opt = AdvanceCursor);

    FileBufferHelper() : pos(0), allocatedSize(0), allocated(0) {}
    ~FileBufferHelper();

    private:
        int pos;
        int allocatedSize;
        char* allocated;
};
