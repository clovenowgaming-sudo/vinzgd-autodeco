#include "AutoDecoPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

namespace autodeco {

    static constexpr int kPopupTag = 0x4150DEC1;

    static std::vector<std::string> const kThemeOrder = {
        "auto", "neon", "toxic", "fire", "ice", "space", "monochrome", "cosmic_moon", "retrowave"
    };

    AutoDecoPopup* AutoDecoPopup::create(LevelEditorLayer* lel, EditorUI* ui) {
        auto ret = new AutoDecoPopup();
        if (ret && ret->init(lel, ui)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void AutoDecoPopup::showOn(cocos2d::CCNode* parent) {
        if (!parent) return;
        if (auto existing = parent->getChildByTag(kPopupTag)) {
            existing->removeFromParent();
        }
        this->setTag(kPopupTag);
        parent->addChild(this, 1000);
    }

    bool AutoDecoPopup::init(LevelEditorLayer* lel, EditorUI* ui) {
        m_lel = lel;
        m_ui = ui;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Full-screen dim background that eats touches so clicks don't
        // fall through to the editor underneath.
        if (!CCLayerColor::initWithColor({0, 0, 0, 160}, winSize.width, winSize.height)) {
            return false;
        }
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);

        m_selectedTheme = Mod::get()->getSettingValue<std::string>("theme");

        float panelW = 360.f, panelH = 260.f;
        float centerX = winSize.width / 2.f;
        float centerY = winSize.height / 2.f;

        m_panel = CCLayerColor::create({20, 20, 30, 235}, panelW, panelH);
        m_panel->setPosition({centerX - panelW / 2.f, centerY - panelH / 2.f});
        m_panel->setAnchorPoint({0.f, 0.f});
        this->addChild(m_panel);

        auto border = CCLayerColor::create({120, 90, 220, 255}, panelW, panelH);
        border->setPosition(m_panel->getPosition() - ccp(2.f, 2.f));
        border->setContentSize({panelW + 4.f, panelH + 4.f});
        this->addChild(border, -1);

        auto title = CCLabelBMFont::create("AutoDeco Beat Sync", "goldFont.fnt");
        title->setScale(0.6f);
        title->setPosition({panelW / 2.f, panelH - 20.f});
        m_panel->addChild(title);

        auto closeBtnSpr = ButtonSprite::create("X", "bigFont.fnt", "GJ_button_06.png", 0.6f);
        auto closeBtn = CCMenuItemSpriteExtra::create(closeBtnSpr, this, menu_selector(AutoDecoPopup::onClose));
        closeBtn->setPosition({panelW - 20.f, panelH - 20.f});
        auto closeMenu = CCMenu::create();
        closeMenu->addChild(closeBtn);
        closeMenu->setPosition({0.f, 0.f});
        m_panel->addChild(closeMenu);

        float topY = panelH - 50.f;

        auto makeRow = [&](char const* labelText, float y, std::string const& defaultVal) -> CCTextInputNode* {
            auto label = CCLabelBMFont::create(labelText, "bigFont.fnt");
            label->setScale(0.35f);
            label->setAnchorPoint({1.f, 0.5f});
            label->setPosition({panelW / 2.f - 10.f, y});
            m_panel->addChild(label);

            auto input = CCTextInputNode::create(100.f, 30.f, "0", "bigFont.fnt");
            input->setPosition({panelW / 2.f + 55.f, y});
            input->setString(defaultVal.c_str());
            m_panel->addChild(input);
            return input;
        };

        m_bpmInput = makeRow("BPM:", topY, std::to_string(Mod::get()->getSettingValue<double>("bpm")));
        m_lengthInput = makeRow("Length (s):", topY - 36.f, std::to_string(Mod::get()->getSettingValue<double>("song-length-seconds")));
        m_offsetInput = makeRow("Offset (ms):", topY - 72.f, std::to_string(Mod::get()->getSettingValue<double>("offset-ms")));
        m_startXInput = makeRow("Start X:", topY - 108.f, std::to_string(Mod::get()->getSettingValue<double>("start-x")));

        // Theme cycle button
        auto themeLabel = CCLabelBMFont::create("Theme:", "bigFont.fnt");
        themeLabel->setScale(0.35f);
        themeLabel->setAnchorPoint({1.f, 0.5f});
        themeLabel->setPosition({panelW / 2.f - 10.f, topY - 144.f});
        m_panel->addChild(themeLabel);

        m_themeLabel = CCLabelBMFont::create("auto", "goldFont.fnt");
        m_themeLabel->setScale(0.45f);
        m_themeLabel->setPosition({panelW / 2.f + 55.f, topY - 144.f});
        m_panel->addChild(m_themeLabel);
        refreshThemeLabel();

        auto cycleBtnSpr = ButtonSprite::create(">", "bigFont.fnt", "GJ_button_01.png", 0.6f);
        auto cycleBtn = CCMenuItemSpriteExtra::create(
            cycleBtnSpr, this, menu_selector(AutoDecoPopup::onCycleTheme)
        );
        cycleBtn->setPosition({panelW / 2.f + 120.f, topY - 144.f});
        auto cycleMenu = CCMenu::create();
        cycleMenu->addChild(cycleBtn);
        cycleMenu->setPosition({0.f, 0.f});
        m_panel->addChild(cycleMenu);

        // Generate button
        auto genBtnSpr = ButtonSprite::create("Generate Deco", "bigFont.fnt", "GJ_button_02.png", 0.9f);
        auto genBtn = CCMenuItemSpriteExtra::create(
            genBtnSpr, this, menu_selector(AutoDecoPopup::onGenerate)
        );
        genBtn->setPosition({panelW / 2.f, 40.f});
        auto genMenu = CCMenu::create();
        genMenu->addChild(genBtn);
        genMenu->setPosition({0.f, 0.f});
        m_panel->addChild(genMenu);

        m_statusLabel = CCLabelBMFont::create("Isi BPM/panjang lagu, pilih tema, lalu Generate.", "chatFont.fnt");
        m_statusLabel->setScale(0.5f);
        m_statusLabel->setPosition({panelW / 2.f, 20.f});
        m_panel->addChild(m_statusLabel);

        return true;
    }

    bool AutoDecoPopup::ccTouchBegan(CCTouch*, CCEvent*) {
        // Swallow every touch on the dim background so clicks don't pass
        // through to the editor underneath while this panel is open.
        return true;
    }

    void AutoDecoPopup::onClose(CCObject*) {
        this->removeFromParent();
    }

    void AutoDecoPopup::refreshThemeLabel() {
        if (m_themeLabel) m_themeLabel->setString(m_selectedTheme.c_str());
    }

    void AutoDecoPopup::onCycleTheme(CCObject*) {
        auto it = std::find(kThemeOrder.begin(), kThemeOrder.end(), m_selectedTheme);
        size_t idx = (it == kThemeOrder.end()) ? 0 : (std::distance(kThemeOrder.begin(), it) + 1) % kThemeOrder.size();
        m_selectedTheme = kThemeOrder[idx];
        refreshThemeLabel();
    }

    static double parseOr(std::string const& s, double fallback) {
        try {
            size_t pos = 0;
            double v = std::stod(s, &pos);
            return v;
        } catch (...) {
            return fallback;
        }
    }

    void AutoDecoPopup::onGenerate(CCObject*) {
        GenerateParams params;
        params.bpm = static_cast<float>(parseOr(m_bpmInput->getString(), 128.0));
        params.songLengthSeconds = static_cast<float>(parseOr(m_lengthInput->getString(), 60.0));
        params.offsetMs = static_cast<float>(parseOr(m_offsetInput->getString(), 0.0));
        params.startX = static_cast<float>(parseOr(m_startXInput->getString(), 0.0));
        params.unitsPerSecond = static_cast<float>(Mod::get()->getSettingValue<double>("units-per-second"));
        params.heightRange = static_cast<float>(Mod::get()->getSettingValue<double>("deco-height-range"));
        params.themeKey = m_selectedTheme;
        params.autoColorMatch = Mod::get()->getSettingValue<bool>("auto-color-match");
        params.varyScaleRotation = Mod::get()->getSettingValue<bool>("vary-scale-rotation");

        auto densityStr = Mod::get()->getSettingValue<std::string>("beat-density");
        if (densityStr == "every-half-beat") params.density = BeatDensity::EveryHalfBeat;
        else if (densityStr == "every-2-beats") params.density = BeatDensity::Every2Beats;
        else if (densityStr == "every-4-beats") params.density = BeatDensity::Every4Beats;
        else params.density = BeatDensity::EveryBeat;

        auto result = generate(m_lel, m_ui, params);

        if (!result.ok) {
            m_statusLabel->setString(result.error.c_str());
            return;
        }

        std::string msg = "Done: " + std::to_string(result.objectsPlaced) + " objek (" + result.themeUsed + ")";
        m_statusLabel->setString(msg.c_str());
        Notification::create(msg, NotificationIcon::Success)->show();
    }

}
