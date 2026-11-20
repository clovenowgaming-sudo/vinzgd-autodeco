#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include "AutoDecoGenerator.hpp"

using namespace geode::prelude;

namespace autodeco {

    // Small confirmation panel shown when the user taps the AutoDeco
    // button in the editor. Built as a plain CCLayerColor modal instead
    // of Geode's Popup<...> helper, to keep this file's dependency
    // surface to a handful of stable, long-standing cocos2d/GD classes
    // (CCLayerColor, CCMenu, CCMenuItemSpriteExtra, CCTextInputNode,
    // ButtonSprite) rather than an SDK-version-specific popup API.
    class AutoDecoPopup : public CCLayerColor {
    protected:
        LevelEditorLayer* m_lel = nullptr;
        EditorUI* m_ui = nullptr;
        CCLayer* m_panel = nullptr;

        CCTextInputNode* m_bpmInput = nullptr;
        CCTextInputNode* m_lengthInput = nullptr;
        CCTextInputNode* m_offsetInput = nullptr;
        CCTextInputNode* m_startXInput = nullptr;

        CCLabelBMFont* m_themeLabel = nullptr;
        CCLabelBMFont* m_statusLabel = nullptr;

        std::string m_selectedTheme = "auto";

        bool init(LevelEditorLayer* lel, EditorUI* ui);

        void onGenerate(CCObject* sender);
        void onCycleTheme(CCObject* sender);
        void onClose(CCObject* sender);
        void refreshThemeLabel();

    public:
        static AutoDecoPopup* create(LevelEditorLayer* lel, EditorUI* ui);

        // Adds this as a full-screen modal child of `parent` (usually the
        // EditorUI itself) and removes any existing instance first.
        void showOn(cocos2d::CCNode* parent);

        bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    };

}
