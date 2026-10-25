# Static recompilation of FishTank2: the demo (gen/: Fishtank2 as its own
# decruncher leaves it at &8000, ../tools/unpack.py) and QTMTracker, the
# music player !Run RMLoads (genqtm/, at &1800000), from SuperSpider's
# analyse and gen, and game.c, linked with libroger (the RISC OS runtime).
ROGER   ?= $(HOME)/stuff/libroger
CC      ?= gcc
CFLAGS  ?= -O2 -g -w -fno-strict-aliasing $(if $(TRACE),-DRECOMP_TRACE)
GEN     := $(wildcard gen/code*.c) gen/entries.c
GENQTM  := $(wildcard genqtm/code*.c) genqtm/entries.c
OBJ     := $(GEN:.c=.o) $(GENQTM:.c=.o) game.o
LIBS    := $(shell pkg-config --libs sdl3) -ljpeg -lm -lpthread
INC     := -I$(ROGER)/include

fishtank2: $(OBJ) $(ROGER)/libroger.a
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(ROGER)/libroger.a $(LIBS)

$(ROGER)/libroger.a: FORCE
	$(MAKE) -C $(ROGER) libroger.a

%.o: %.c $(ROGER)/include/roger.h $(ROGER)/include/roger_cpu.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Code/data analysis: SuperSpider, into ../analysis/<name>.json, then
# superspider gen. Each image's ../analysis/<name>.roots adds entry points
# to the analysis: "make roots" merges the gap entries a --trace-gaps run
# logged (untranslated.log, or LOG=...) into them, so the next "make regen"
# translates them.
SS      ?= $(HOME)/stuff/superspider
SSBIN   := $(SS)/_build/default/bin/main.exe
AN      := ../analysis
APP     := ../game/!FishTank2
GAME    := ../unpacked/fishtank2
QTM     := $(firstword $(wildcard $(APP)/QTMTracker,*) $(APP)/QTMTracker)
QTMBASE := 1800000
LOG     ?= untranslated.log
# --no-smc: neither image modifies its code (the demo's stack, which grows
# up from &8000, only runs over its start-up code, which doesn't run again),
# so the translations don't check each instruction for stores over it

# --roots for ../analysis/<name>.roots, if there is one
roots = $(if $(wildcard $(AN)/$(1).roots),--roots $(AN)/$(1).roots)

regen:
	cd $(SS) && dune build
	python3 -I ../tools/unpack.py
	mkdir -p $(AN)
	$(SSBIN) analyse $(GAME) 8000 --full $(call roots,game) -o $(AN)/game.json
	$(SSBIN) analyse "$(QTM)" $(QTMBASE) --module --full $(call roots,qtm) -o $(AN)/qtm.json
	rm -rf gen genqtm
	$(SSBIN) gen $(GAME) 8000 $(AN)/game.json --c gen --leaders=dispatch --no-smc
	$(SSBIN) gen "$(QTM)" $(QTMBASE) $(AN)/qtm.json --c genqtm --leaders=dispatch --no-smc \
	    --prefix=Q --tabprefix=recompQ --files=1 --covtag=Q

roots:
	python3 -I $(SS)/tools/gap_roots.py $(GAME) 8000 $(AN)/game.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(QTM)" $(QTMBASE) $(AN)/qtm.roots $(LOG)

clean:
	rm -f gen/*.o genqtm/*.o game.o fishtank2

.PHONY: FORCE regen roots clean
