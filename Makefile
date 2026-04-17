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

# Assets are compiled from src/assets.c for C symbol access. Exclude generated
# assembler variants (with or without src/ prefixes) to avoid duplicate
# symbol definitions at link time.
SFILES := $(filter-out assets.asm assets.asp assets.ps $(SRC)/assets.asm $(SRC)/assets.asp $(SRC)/assets.ps,$(SFILES))
OFILES := $(filter-out assets.obj $(SRC)/assets.obj,$(OFILES))

all: $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanAudio
