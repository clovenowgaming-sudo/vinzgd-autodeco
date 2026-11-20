# AutoDeco Beat Sync

Adds an **AD** button in the level editor that auto-generates decoration
objects spaced out to match a song's BPM, with a theme (neon, toxic, fire,
ice, space, monochrome, cosmic moon/gears, retrowave — or auto-detected
from your level's current background color) applied as a color tint and
object palette.

## How to use
1. Open a level in the editor.
2. Tap **AD** in the top-right of the editor screen.
3. Fill in / confirm BPM, song length (seconds), beat offset, and start X.
4. Pick a theme (or leave it on `auto`).
5. Tap **Generate Deco**.

## Settings
Tune defaults (BPM, density, color matching, scale/rotation variation,
height spread, units-per-second for the active speed portal) from the
mod's settings page so you don't have to retype them every time.

## Notes
- This places decoration as plain objects with a flat color tint — it
  does not wire them into the level's color-trigger channels.
- "auto" theme detection is a simple color-hue guess from the editor's
  current background color; override it manually if it guesses wrong.
- Units/second assumes Normal Speed (1x) by default — change the
  "Editor Units / Second" setting to match whatever speed portal is
  active in the part you're decorating.
