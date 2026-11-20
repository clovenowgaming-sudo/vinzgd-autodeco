#include "AutoDecoGenerator.hpp"
#include <Geode/binding/GJEffectManager.hpp>
#include <cmath>
#include <algorithm>

using namespace geode::prelude;

namespace autodeco {

    // ------------------------------------------------------------------
    // Palette. The object IDs here are a starting point copied from
    // common free-to-use decoration blocks in the base game's object
    // list. GD's object IDs can differ slightly between 2.2 sub-versions
    // and some of these may not be the exact block you expect visually —
    // open the in-game Build menu, find blocks you like for each theme,
    // and swap the numbers below (or override per-call once a settings
    // UI for it exists). Nothing here was verified against a live build.
    // ------------------------------------------------------------------
    static std::vector<DecoTheme> const THEMES = {
        DecoTheme{
            "neon", "Neon / Modern",
            /* small */   {211, 212, 213, 1202, 1203},
            /* big */      {1278, 1279, 1280},
            /* particle */ {1704, 1705, 1931}
        },
        DecoTheme{
            "toxic", "Toxic / Acid",
            {367, 368, 369, 1202},
            {1517, 1518, 1519},
            {1704, 1931}
        },
        DecoTheme{
            "fire", "Fire / Lava",
            {506, 507, 508, 1202},
            {1543, 1544, 1545},
            {1932, 1933}
        },
        DecoTheme{
            "ice", "Ice / Crystal",
            {1163, 1164, 1165, 1202},
            {1598, 1599, 1600},
            {1704, 1934}
        },
        DecoTheme{
            "space", "Space / Void",
            {1278, 1279, 1202},
            {1649, 1650, 1651},
            {1931, 1935}
        },
        DecoTheme{
            "monochrome", "Monochrome / Minimal",
            {211, 212, 1202},
            {1278, 1279},
            {1704}
        },
        DecoTheme{
            // Matches the glowing-moon / gears / angular crystal shard look —
            // dark jagged pillar shapes, spinning gear decos, and a scatter
            // of small glow/star particles in blue-purple tones.
            "cosmic_moon", "Cosmic Moon / Gears",
            /* small: gear + small glow accents */ {1202, 1203, 1598, 1599},
            /* big: angular crystal/shard pillars */ {1649, 1650, 1651, 1600},
            /* particle: stars / glow motes */ {1704, 1705, 1931, 1935}
        },
        DecoTheme{
            // Synthwave bonus theme: pink/purple grid glow, sharp triangles.
            "retrowave", "Retrowave / Synth Grid",
            {1202, 1203, 1278},
            {1279, 1280, 1517},
            {1704, 1931, 1932}
        }
    };

    std::vector<DecoTheme> const& getThemes() {
        return THEMES;
    }

    DecoTheme const* findTheme(std::string const& key) {
        for (auto& t : THEMES) {
            if (t.key == key) return &t;
        }
        return nullptr;
    }

    // RGB -> hue bucket. Cheap, no external color lib needed.
    static float hueOf(ccColor3B c) {
        float r = c.r / 255.f, g = c.g / 255.f, b = c.b / 255.f;
        float mx = std::max({r, g, b}), mn = std::min({r, g, b});
        float d = mx - mn;
        if (d < 0.0001f) return -1.f; // greyscale, no real hue
        float h;
        if (mx == r) h = fmodf((g - b) / d, 6.f);
        else if (mx == g) h = (b - r) / d + 2.f;
        else h = (r - g) / d + 4.f;
        h *= 60.f;
        if (h < 0) h += 360.f;
        return h;
    }

    std::string detectThemeFromLevel(LevelEditorLayer* lel) {
        if (!lel) return "neon";

        // Read the editor's current background color straight from the
        // level's effect manager (channel 1000 = background). This is the
        // same place color triggers and the BG color picker write to.
        if (!lel->m_effectManager) return "neon";
        ccColor3B bg = lel->m_effectManager->colorForIndex(1000);

        float h = hueOf(bg);
        float brightness = (bg.r + bg.g + bg.b) / (3.f * 255.f);

        if (h < 0.f || brightness < 0.12f) return "monochrome";
        if (h < 20.f || h >= 345.f)  return "fire";
        if (h < 50.f)                return "fire";
        if (h < 160.f)               return "toxic";
        if (h < 210.f)               return "ice";
        if (h < 265.f)               return "space";
        return "neon";
    }

    static int pickFrom(std::vector<int> const& v, int idx) {
        if (v.empty()) return 211; // fallback: basic block, always exists
        return v[idx % v.size()];
    }

    GenerateResult generate(LevelEditorLayer* lel, EditorUI* ui, GenerateParams const& params) {
        GenerateResult result;

        if (!lel || !ui) {
            result.error = "Editor tidak terbuka.";
            return result;
        }
        if (params.bpm <= 0.f || params.songLengthSeconds <= 0.f) {
            result.error = "BPM dan panjang lagu harus lebih dari 0.";
            return result;
        }

        std::string themeKey = params.themeKey;
        if (themeKey == "auto" || themeKey.empty()) {
            themeKey = detectThemeFromLevel(lel);
        }
        auto* theme = findTheme(themeKey);
        if (!theme) theme = &getThemes().front();
        result.themeUsed = theme->displayName;

        float secondsPerBeat = 60.f / params.bpm;
        switch (params.density) {
            case BeatDensity::EveryHalfBeat: secondsPerBeat *= 0.5f; break;
            case BeatDensity::Every2Beats:   secondsPerBeat *= 2.f;  break;
            case BeatDensity::Every4Beats:   secondsPerBeat *= 4.f;  break;
            case BeatDensity::EveryBeat:     default: break;
        }

        int totalBeats = static_cast<int>(std::floor(params.songLengthSeconds / secondsPerBeat));
        // Hard safety cap so a long song / tight density doesn't try to spawn
        // tens of thousands of objects and freeze the editor.
        constexpr int kMaxObjects = 4000;
        totalBeats = std::min(totalBeats, kMaxObjects);

        float groundBaselineY = 105.f; // default ground top in editor Y units (2.2 default ground)

        // Figure out a color to tint decoration with, from the editor's
        // current background colour (same source as detectThemeFromLevel).
        // This is a flat tint on the object's sprite, not a proper color
        // channel/trigger — good enough visually, but it won't react to
        // in-level color triggers the way channel-linked objects would.
        ccColor3B tint{255, 255, 255};
        if (params.autoColorMatch && lel->m_effectManager) {
            ccColor3B bg = lel->m_effectManager->colorForIndex(1000);
            // Push toward a brighter, more saturated version of the bg hue
            // so deco reads as an accent rather than blending invisibly in.
            tint.r = static_cast<GLubyte>(std::min(255, bg.r + 60));
            tint.g = static_cast<GLubyte>(std::min(255, bg.g + 60));
            tint.b = static_cast<GLubyte>(std::min(255, bg.b + 60));
        }

        int placed = 0;
        for (int i = 0; i < totalBeats; ++i) {
            float t = (params.offsetMs / 1000.f) + i * secondsPerBeat;
            if (t < 0.f) continue;
            float x = params.startX + t * params.unitsPerSecond;

            bool isStrongBeat = (i % 4 == 0);
            int objID = isStrongBeat
                ? pickFrom(theme->bigObjIDs, i / 4)
                : pickFrom(theme->smallObjIDs, i);

            float yJitter = params.heightRange > 0.f
                ? (CCRANDOM_0_1() * params.heightRange)
                : 0.f;
            float y = groundBaselineY + yJitter;

            auto* obj = ui->createObject(objID, ccp(x, y));
            if (!obj) continue;

            if (params.varyScaleRotation) {
                float scale = 0.85f + CCRANDOM_0_1() * 0.4f; // 0.85x - 1.25x
                float rot = (CCRANDOM_0_1() - 0.5f) * 20.f;  // +-10 degrees
                obj->setScale(scale);
                obj->setRotation(rot);
            }
            if (params.autoColorMatch) {
                obj->setColor(tint);
            }

            // Every ~8 beats, sprinkle a floatier background particle piece
            // higher up so the deco fills more than just the ground line.
            if (i % 8 == 0 && !theme->particleObjIDs.empty()) {
                float py = groundBaselineY + params.heightRange * (0.6f + CCRANDOM_0_1() * 0.4f);
                auto* particle = ui->createObject(pickFrom(theme->particleObjIDs, i / 8), ccp(x, py));
                if (particle) {
                    if (params.autoColorMatch) particle->setColor(tint);
                    particle->setScale(0.7f + CCRANDOM_0_1() * 0.5f);
                    placed++;
                }
            }

            placed++;
        }

        result.objectsPlaced = placed;
        result.ok = true;
        return result;
    }

}
