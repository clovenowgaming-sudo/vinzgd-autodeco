#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include "AutoDecoGenerator.hpp"

using namespace geode::prelude;

namespace autodeco {

    // Small confirmation popup shown when the user taps the AutoDeco
    // button in the editor. Lets them double check BPM / length / theme
    // (pre-filled from mod settings) before anything gets placed, since
    // this can add a lot of objects at once.
    class AutoDecoPopup : public geode::Popup<LevelEditorLayer*, EditorUI*> {
    protected:
        LevelEditorLayer* m_lel = nullptr;
        EditorUI* m_ui = nullptr;

        CCTextInputNode* m_bpmInput = nullptr;
        CCTextInputNode* m_lengthInput = nullptr;
        CCTextInputNode* m_offsetInput = nullptr;
        CCTextInputNode* m_startXInput = nullptr;

        CCLabelBMFont* m_themeLabel = nullptr;
        CCLabelBMFont* m_statusLabel = nullptr;

        std::string m_selectedTheme = "auto";

        bool setup(LevelEditorLayer* lel, EditorUI* ui) override;

        void onGenerate(CCObject* sender);
        void onCycleTheme(CCObject* sender);
        void refreshThemeLabel();

    public:
        static AutoDecoPopup* create(LevelEditorLayer* lel, EditorUI* ui);
    };

}
