/*!
 * \file LevelEditor.h
 * \brief A editor to modify level in game
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once


/*!
 * \class LevelEditor
 * \brief A editor to modify level in game
 */
class LevelEditor {
    public:
        LevelEditor();
        ~LevelEditor();

        /*!
         * \brief Update function which makes actions
         * \param dt : time since the last call
         */
        void tick(float dt);

    private:
        struct LevelEditorDatas;
        LevelEditorDatas* datas;
};
