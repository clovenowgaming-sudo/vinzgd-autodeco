#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include "AutoDecoPopup.hpp"

using namespace geode::prelude;
using namespace autodeco;

class $modify(AutoDecoEditorUI, EditorUI) {
    // Keep a tag so we don't duplicate the button if init somehow runs twice.
    static constexpr int kButtonTag = 0x4150DEC0; // arbitrary, "APDECO"-ish

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        if (this->getChildByTag(kButtonTag)) {
            return true;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto btnSpr = ButtonSprite::create("AD", "bigFont.fnt", "GJ_button_04.png", 0.8f);
        auto btn = CCMenuItemSpriteExtra::create(
            btnSpr, this, menu_selector(AutoDecoEditorUI::onOpenAutoDeco)
        );

        auto menu = CCMenu::create();
        menu->addChild(btn);
        // Top-left-ish corner, out of the way of the normal build/edit bars.
        menu->setPosition({winSize.width - 40.f, winSize.height - 120.f});
        menu->setTag(kButtonTag);
        menu->setID("autodeco-button-menu"_spr);
        this->addChild(menu, 100);

        return true;
    }

    void onOpenAutoDeco(CCObject*) {
        auto lel = LevelEditorLayer::get();
        if (!lel) {
            Notification::create("Editor belum siap.", NotificationIcon::Error)->show();
            return;
        }
        auto popup = AutoDecoPopup::create(lel, this);
        if (popup) {
            popup->showOn(this);
        }
    }
};
