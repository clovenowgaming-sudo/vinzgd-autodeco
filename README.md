# AutoDeco Beat Sync (Geode mod)

Target: **Geometry Dash 2.2081**, **Geode SDK v4.3.0** (match the SDK you
already use for your geode-llm project — change the `geode`/`gd` fields in
`mod.json` if you're on a different version).

## Build

Needs the Geode CLI + the actual GD/Geode SDK headers + a C++ toolchain for
your target platform (Android NDK, or `geode sdk install-linux` for the
Windows cross-toolchain on Linux). That's generally **not available in a
bare Termux shell** — so build this on a PC, in CI, or in any environment
that has the Geode SDK installed, then copy the resulting `.geode` file to
your phone's `Android/data/com.geode.launcher/files/game/geode/mods/`
folder (exact path depends on your Geode installation — check the Geode
launcher's own docs if unsure).

```sh
export GEODE_SDK=/path/to/geode-sdk
geode build   # or: cmake -B build && cmake --build build
```

### Building via GitHub Actions (no local toolchain needed)

`.github/workflows/build.yml` is included and builds Windows/macOS/Android32/
Android64 on every push, using the community `geode-sdk/build-geode-mod`
action to set up the SDK on GitHub's runners — same idea as the GitHub
Actions CI you already use for your Kotlin/Android builds, just pointed at
Geode instead.

To use it from Termux:
```sh
cd vinzgd-autodeco
git init && git add . && git commit -m "init"
gh repo create vinzgd-autodeco --public --source=. --push   # needs `gh auth login` once
```
Then check the **Actions** tab on GitHub — each run uploads the built
`.geode` files as downloadable artifacts (one per platform) under that run.
Grab the `android32`/`android64` one, put it in your Geode mods folder, done.

If `geode-sdk/build-geode-mod` ever renames an input or stops existing,
the job will just fail with a clear "action not found"/"invalid input"
error in the Actions log — swap in whatever the current official Geode CI
action/README recommends at that point.

## What's verified vs. what isn't

Written by hand against Geode's public API (`$modify`, `EditorUI`,
`CCMenuItemSpriteExtra`, `geode::Popup`, settings schema) — these patterns
are standard and should build cleanly. Three things are **not** verified
against a live SDK/build and you should sanity-check them before relying
on the result:

1. **Decoration object IDs** in `AutoDecoGenerator.cpp` (`THEMES`) — these
   are placeholders. Open the in-game Build menu, pick blocks you actually
   like for each theme, and replace the ID numbers.
2. **`lel->m_backgroundLayer`** (used for auto theme-detect and color
   match) — this is the commonly-referenced background layer member on
   `GJBaseGameLayer`, but field names can shift between Geode binding
   versions. If it doesn't compile, either update the member name to match
   your installed bindings, or just delete the auto-detect block — theme
   selection still works fine manually from the popup.
3. **Units-per-second (311.58 default)** — this is the commonly-cited GD
   "Normal Speed" constant for converting seconds → editor X units. If
   your part uses a different speed portal, change the
   "Editor Units / Second" setting accordingly (e.g. roughly double it for
   Double Speed) or the deco will drift off the beat.

None of this was run against a real build — I don't have the GD binary,
Geode SDK, or NDK in this environment to compile and test it. Treat it as
a complete, consistent starting project rather than a confirmed-working
one, and iterate from your first build/test pass.
