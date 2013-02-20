#pragma once

class LevelEditor {
    public:
        LevelEditor();
        ~LevelEditor();
        void tick(float dt);

        static void lock();
        static void unlock();
    private:
        struct LevelEditorDatas;
        LevelEditorDatas* datas;
};
