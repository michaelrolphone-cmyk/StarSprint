# StarSprint SNES

A PVSnesLib homebrew side-scrolling platformer prototype for SNES.

## Current campaign build

- 3 worlds
- 4 levels per world
- 2-player shared-screen co-op in every level
- title screen plus interactive world map with level selection and progression
- hidden grow and lightning powerups
- smiley gold stars for score and boost banking
- 60 stars = 1 minute of banked super speed
- stomp enemies to defeat them and bounce
- smash brick blocks from below or by down-smashing in the air
- bouncing lightning bolts once the lightning form is collected
- faster movement, tuned jump height, more forgiving small-form tunnel movement, and enemies that patrol cliff edges instead of falling off
- rope swing hazards, with airborne grab and timed release for extra traversal challenges

## Controls

### Title screen
- Start or A: go to the world map

### World map
- D-pad: move between unlocked levels
- A or Start: enter selected level

### In level
- Player 1 uses controller 1, player 2 uses controller 2
- Left / Right: move
- B: jump / release from a rope swing
- Hold A: run faster
- Hold A with banked boost: consume boost and run at super speed
- Press A while lightning power is active: throw a lightning bolt
- Press Y near your teammate: pick them up
- Press Y again while holding: throw teammate with jump-style arc dynamics
- If held, press B to jump free and escape
- Down while falling: smash through brick blocks underfoot

## Build

Set `PVSNESLIB_HOME` to your extracted PVSnesLib directory, then run:

```sh
make clean
make
```

The ROM output is `starsprint.sfc`.

## Test

Run the timing regression tests with:

```sh
python3 -m unittest discover -s tests -p 'test_*.py'
```
