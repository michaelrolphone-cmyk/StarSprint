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

## Latest tuning notes

- rope release now has a short no-regrab window so you can actually let go
- rope visuals were changed away from the lightning-bolt look
- play simulation now advances twice per rendered frame to restore the faster game feel
