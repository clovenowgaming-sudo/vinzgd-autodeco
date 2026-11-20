#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <vector>
#include <string>

using namespace geode::prelude;

namespace autodeco {

    // One "look" the generator can draw decoration from.
    // IDs below are a starting palette, not a verified final list —
    // see the README note about checking them against the object list
    // for your GD version before relying on the result.
    struct DecoTheme {
        std::string key;
        std::string displayName;
        std::vector<int> smallObjIDs;   // ground-hugging accents, placed every beat
        std::vector<int> bigObjIDs;     // larger set-pieces, placed on strong/accented beats
        std::vector<int> particleObjIDs; // floaty background accents for held/background layer
    };

    enum class BeatDensity {
        EveryBeat,
        EveryHalfBeat,
        Every2Beats,
        Every4Beats
    };

    struct GenerateParams {
        float bpm = 128.f;
        float songLengthSeconds = 60.f;
        float offsetMs = 0.f;
        float startX = 0.f;
        float unitsPerSecond = 311.58f;
        float heightRange = 180.f;
        std::string themeKey = "auto";
        BeatDensity density = BeatDensity::EveryBeat;
        bool autoColorMatch = true;
        bool varyScaleRotation = true;
    };

    struct GenerateResult {
        int objectsPlaced = 0;
        std::string themeUsed;
        bool ok = false;
        std::string error;
    };

    // Returns the full built-in theme list (including "auto" resolved separately).
    std::vector<DecoTheme> const& getThemes();

    DecoTheme const* findTheme(std::string const& key);

    // Best-effort guess at a theme from the level's current BG/Ground colors.
    // Falls back to "neon" if nothing can be read.
    std::string detectThemeFromLevel(LevelEditorLayer* lel);

    // Does the actual placement. Must be called on the editor's thread
    // (i.e. from a menu callback), with lel/ui pointing at the currently open editor.
    GenerateResult generate(LevelEditorLayer* lel, EditorUI* ui, GenerateParams const& params);

}
