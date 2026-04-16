ifeq ($(filter test bundle-prebuilt,$(MAKECMDGOALS)),)
ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Please create an environment variable PVSNESLIB_HOME by following this guide: https://github.com/alekmaul/pvsneslib/wiki/Installation")
endif

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules
endif

.PHONY: all clean test bundle bundle-prebuilt

export ROMNAME := starsprint
SRC := ./src

# Prevent the support include from being treated as a build unit.
SFILES := $(filter-out hdr.asm,$(SFILES))
OFILES := $(filter-out hdr.obj,$(OFILES))

all: $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanAudio

test:
	cc -std=c99 -Wall -Wextra -pedantic tests/test_video_layout.c -o tests/test_video_layout
	./tests/test_video_layout
	cc -std=c99 -Wall -Wextra -pedantic tests/test_sprite_format.c src/sprite_format.c src/assets.c -o tests/test_sprite_format
	./tests/test_sprite_format
	cc -std=c99 -Wall -Wextra -pedantic tests/test_game_logic.c src/game_logic.c -o tests/test_game_logic
	./tests/test_game_logic
	./tests/test_bundle_prebuilt.sh

bundle: all bundle-prebuilt

bundle-prebuilt:
	@test -f $(ROMNAME).sfc || (echo "Missing $(ROMNAME).sfc. Build first with 'make'." && exit 1)
	mkdir -p dist
	cp $(ROMNAME).sfc dist/$(ROMNAME).sfc
