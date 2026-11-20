#include "AutoDecoPopup.hpp"
#include <Geode/binding/ButtonSprite.hpp>
#include <vector>

using namespace geode::prelude;

namespace autodeco {

    static std::vector<std::string> const kThemeOrder = {
        "auto", "neon", "toxic", "fire", "ice", "space", "monochrome", "cosmic_moon", "retrowave"
    };

    AutoDecoPopup* AutoDecoPopup::create(LevelEditorLayer* lel, EditorUI* ui) {
        auto ret = new AutoDecoPopup();
        if (ret && ret->initAnchored(360.f, 220.f, lel, ui)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool AutoDecoPopup::setup(LevelEditorLayer* lel, EditorUI* ui) {
        m_lel = lel;
        m_ui = ui;
        this->setTitle("AutoDeco Beat Sync");

        m_selectedTheme = Mod::get()->getSettingValue<std::string>("theme");

        auto winSize = m_mainLayer->getContentSize();
        float centerX = winSize.width / 2.f;
        float topY = winSize.height - 60.f;

        auto makeRow = [&](char const* labelText, float y, std::string const& defaultVal) -> CCTextInputNode* {
            auto label = CCLabelBMFont::create(labelText, "bigFont.fnt");
            label->setScale(0.35f);
            label->setAnchorPoint({1.f, 0.5f});
            label->setPosition({centerX - 10.f, y});
            m_mainLayer->addChild(label);

            auto input = CCTextInputNode::create(100.f, 30.f, "0", "bigFont.fnt");
            input->setPosition({centerX + 55.f, y});
            input->setString(defaultVal.c_str());
            m_mainLayer->addChild(input);
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
        themeLabel->setPosition({centerX - 10.f, topY - 144.f});
        m_mainLayer->addChild(themeLabel);

        m_themeLabel = CCLabelBMFont::create("auto", "goldFont.fnt");
        m_themeLabel->setScale(0.45f);
        m_themeLabel->setPosition({centerX + 55.f, topY - 144.f});
        m_mainLayer->addChild(m_themeLabel);
        refreshThemeLabel();

        auto cycleBtnSpr = ButtonSprite::create(">", "bigFont.fnt", "GJ_button_01.png", 0.6f);
        auto cycleBtn = CCMenuItemSpriteExtra::create(
            cycleBtnSpr, this, menu_selector(AutoDecoPopup::onCycleTheme)
        );
        cycleBtn->setPosition({centerX + 120.f, topY - 144.f});
        auto cycleMenu = CCMenu::create();
        cycleMenu->addChild(cycleBtn);
        cycleMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(cycleMenu);

        // Generate button
        auto genBtnSpr = ButtonSprite::create("Generate Deco", "bigFont.fnt", "GJ_button_02.png", 0.9f);
        auto genBtn = CCMenuItemSpriteExtra::create(
            genBtnSpr, this, menu_selector(AutoDecoPopup::onGenerate)
        );
        genBtn->setPosition({centerX, 40.f});
        auto genMenu = CCMenu::create();
        genMenu->addChild(genBtn);
        genMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(genMenu);

        m_statusLabel = CCLabelBMFont::create("Isi BPM/panjang lagu, pilih tema, lalu Generate.", "chatFont.fnt");
        m_statusLabel->setScale(0.5f);
        m_statusLabel->setPosition({centerX, 70.f});
        m_mainLayer->addChild(m_statusLabel);

        return true;
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
