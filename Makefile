ifneq ($(MAKECMDGOALS),test)
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Please create an environment variable PVSNESLIB_HOME by following this guide: https://github.com/alekmaul/pvsneslib/wiki/Installation")
endif

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules
endif

.PHONY: all clean test

export ROMNAME := starsprint
SRC := ./src

# Prevent the support include from being treated as a build unit.
SFILES := $(filter-out hdr.asm,$(SFILES))
OFILES := $(filter-out hdr.obj,$(OFILES))

all: $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanAudio

test:
	cc -std=c99 -Wall -Wextra -pedantic tests/test_sprite_format.c src/sprite_format.c src/assets.c -o tests/test_sprite_format
	./tests/test_sprite_format
