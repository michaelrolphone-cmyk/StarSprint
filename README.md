# StarSprint SNES

A PVSnesLib homebrew side-scrolling platformer prototype for SNES.

## Current campaign build

- 3 worlds
- 4 levels per world
- title screen plus interactive world map with level selection and progression
- 2-player support with cooperative and turn-based modes
- contextual sound-effect event hooks for movement, combat, pickups, and turn swaps
- hidden grow and lightning powerups
- one hidden extra-life powerup box per level
- smiley gold stars for score and boost banking
- 100 stars = 1 minute of banked super speed
- players start each run with 5 lives
- stomp enemies to defeat them and bounce
- smash brick blocks from below or by down-smashing in the air
- bouncing lightning bolts once the lightning form is collected
- faster movement, tuned jump height, more forgiving small-form tunnel movement, and enemies that patrol cliff edges instead of falling off
- rope swing hazards, with airborne grab and timed release for extra traversal challenges

## Controls

### Title screen
- Select: toggle 2-player mode (Co-op / Turn-based)
- Start or A: go to the world map

### World map
- D-pad: move between unlocked levels
- A or Start: enter selected level

### In level
- Co-op mode: both controllers can control the same active hero at the same time
- Turn-based mode: controller 1 controls Player 1 turns, controller 2 controls Player 2 turns
- Left / Right: move
- B: jump / release from a rope swing
- Hold Y: run faster
- Hold Y with banked boost: consume boost and run at super speed
- Press Y while lightning power is active: throw a lightning bolt
- Down while falling: smash through brick blocks underfoot
- Hit hidden life boxes for an extra life (up to 9)

## Build

Clone and install PVSnesLib from https://github.com/alekmaul/pvsneslib, set `PVSNESLIB_HOME` to that checkout, then run:

```sh
make clean
make
make test
make bundle
```

The ROM output is `starsprint.sfc` (gitignored and not committed).
The bundled ROM output is `dist/starsprint.sfc` (also gitignored).

If you already have `starsprint.sfc`, run this command to package without rebuilding:

```sh
make bundle-prebuilt
```


## Standalone level editor

A dependency-free single-file browser level editor is included at `level-editor/index.html`.

### Level file folder

All editable level files live in `level-editor/levels/`.
- `index.json` is the manifest loaded by the editor.
- Each level is a JSON file listed in that manifest.

### Run the editor

Serve the repository root (or the `level-editor/` folder) with a static server, then open the editor page:

```sh
python3 -m http.server 8000
# then open http://localhost:8000/level-editor/
```

Use **Export JSON** to download a level file, then move it into `level-editor/levels/` and add the filename to `level-editor/levels/index.json`.

## Test commands

Run all unit tests with:

```sh
make test
```

## Latest tuning notes

- title, world-map, and clear-state UI now use themed palettes to better match the cover-art color treatment
- title and world-map scenes now stage sprites in a cover-inspired layout (hero centerpiece, star arc, rocket trail, and decorative pickups)
- sprite palette was retuned for stronger saturated contrast to better reflect the uploaded art direction
- level generation now adds cover-style setpieces (star arches, suspended metallic cloud routes, and extra elevated platform beats) to better match the requested in-level look
- rope release now honors the no-regrab lock window, preventing immediate reattachment on jump release
- the play loop now enables a second simulation substep only in high-motion moments (super speed, high velocity, or active bolts) to reduce per-frame CPU cost
- co-op now merges controller input for shared movement while turn-based mode routes controls to the active player
- turn-based mode persists each player's score/form/boost state and swaps players on wipe or level clear
- gameplay events now trigger sound-effect IDs through a dedicated event mapping layer
