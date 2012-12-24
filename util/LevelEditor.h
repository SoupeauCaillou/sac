#pragma once

class LevelEditor {
    public:
        LevelEditor();
        ~LevelEditor();
        void tick(float dt);

    private:
        struct LevelEditorDatas;
        LevelEditorDatas* datas;
};
