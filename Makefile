ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Please create an environment variable PVSNESLIB_HOME by following this guide: https://github.com/alekmaul/pvsneslib/wiki/Installation")
endif

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: all clean

export ROMNAME := starsprint
SRC := ./src

# Prevent the support include from being treated as a build unit.
SFILES := $(filter-out hdr.asm,$(SFILES))
OFILES := $(filter-out hdr.obj,$(OFILES))

all: $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanAudio
