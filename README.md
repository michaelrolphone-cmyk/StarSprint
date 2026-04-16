# StarSprint SNES

A PVSnesLib homebrew side-scrolling platformer prototype for SNES.

## Current campaign build

- 3 worlds
- 4 levels per world
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
- Left / Right: move
- B: jump / release from a rope swing
- Hold Y: run faster
- Hold Y with banked boost: consume boost and run at super speed
- Press Y while lightning power is active: throw a lightning bolt
- Down while falling: smash through brick blocks underfoot

## Build

Set `PVSNESLIB_HOME` to your extracted PVSnesLib directory, then run:

```sh
make clean
make
make test
```

The ROM output is `starsprint.sfc`.

## Test command

Run unit tests for gameplay helper logic with:

```sh
gcc -std=c99 -Wall -Wextra -pedantic tests/test_game_logic.c src/game_logic.c -o tests/test_game_logic
./tests/test_game_logic
```

## Latest tuning notes

- rope release now honors the no-regrab lock window, preventing immediate reattachment on jump release
- the play loop now enables a second simulation substep only in high-motion moments (super speed, high velocity, or active bolts) to reduce per-frame CPU cost
