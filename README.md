FishTank2 (DFI, 1997) - static recompilation
============================================

A recipe: this repository holds no data from the demo. It rebuilds
FishTank2, Paul Thomson's Risc PC demo for DFI (13 December 1997; music
by Captain/Image), as a native executable from your own copy, recognised
by content (recipe.json), so it can be shared without distributing the
demo.

    ./build.sh DFI
    cd recomp && ./fishtank2

build.sh runs recipe.json's steps with SuperSpider's tools/weave.py
(~/stuff/superspider, or $SUPERSPIDER; $SS below): it extracts the
archive, runs the demo's own decruncher (tools/unpack.py), analyses and
translates the demo and its music player, and links them with libroger
(~/stuff/libroger, or make ROGER=...). Needs python3, OCaml/dune, gcc,
SDL3 and, for libroger, stb_image (Debian's libstb-dev).

The input: DFI's demo collection on Acorn User's CD 6 (December 1998),
the Spark archive EVOLA.CLASSICS.MODERN.DFI (1630388 bytes, sha256
c3494a80...), copied off the CD - it holds !FishTank2 among DFI's other
demos. A zip or directory of !FishTank2 itself works too (its files are
the recipe's key files).

What it makes:

    extracted/dfi         the archive, via weave.py
    game/!FishTank2       libroger's appdir (../game)
    unpacked/fishtank2    tools/unpack.py: Fishtank2 as its decruncher leaves
                          it, at &8000
    analysis/*.json       SuperSpider's analyses (game, qtm)
    recomp/gen*/          their C translations
    recomp/fishtank2      the demo

Committed: recipe.json, build.sh, tools/unpack.py, recomp/Makefile,
recomp/game.c, recomp/fishtank2.ini, this file.

Watching
--------

    ./fishtank2                 the demo, start to end (about four and a
                                half minutes), then back to the "desktop"
                                (it ends with *Desktop: the run ends)
    ./fishtank2 --crop          as fishtank2.ini says: all 288 rows

Escape fades out and ends it early. It draws as fast as the CPU allows,
one frame per vsync at most (libroger's vsync is 50Hz; the demo's own mode
is 86Hz), and follows the music for its sequence; interactive runs are
unthrottled. Headless runs (--limit) count time at a StrongARM's 200
million instructions a second (game.c's headless_mips), so they show as
many frames as a Risc PC would: the whole demo is some 54 billion
instructions (--limit=6e10; a few minutes).

How the demo is put together
----------------------------

!Run asks for 6700K of application memory, RMLoads QTMTracker (QTM Tracker
1.27, Phoenix/Quantum: the music player) and runs Fishtank2, an absolute
packed with a decruncher on its end: the first word branches to &80208,
which copies the decruncher above where the unpacked image will end
(OS_SynchroniseCodeAreas on the copy), decrunches the program backwards to
&8000-&A4D5C and enters it at &8000. tools/unpack.py runs that under
$SS/tools/arm2.py; the image it leaves is what's translated.

The demo keeps an ascending stack from &8000 (stmia sp! / ldmdb sp!) that
grows over its own start-up code (stored-over words that never run
again), and several routines save SP in a literal word and use it as a
data register in their inner loops. It checks that it can have 1008K of
screen memory (two banks of its mode) and takes it (OS_ChangeDynamicArea),
selects a 448x288 mode at 32 bits per pixel with a mode selector (ModeFile
is the monitor definition !Help asks you to merge: 86Hz), builds its tables
(a sine table in floating point - FPA instructions, the FPEmulator's - and
a reciprocal table that takes most of the start-up's 480 million
instructions), plots its twelve JPEG textures with SpriteExtend
(JPEG_PlotScaled) and copies them back off the screen, then starts the
music from memory (QTM_Load, QTM_Start) and runs its parts - lens flares
over a JPEG sky, a voxel landscape with flying fish, a tunnel, the credits
- each frame drawn into the hidden bank, shown with OS_Byte 113 and a wait
for vsync, fading through VIDC20's gamma tables (PaletteV 9).

QTMTracker plays from TickerV and fills the sound DMA buffers with a Level
1 handler of its own (Sound_Configure: 4 channels, 32us, 208 samples). On
RISC OS 3.5 and later it takes its sample rates from VIDC's clock (VDU
variable 172).

The translation: the demo (gen/, from &8000) and QTMTracker (genqtm/, a
module at &1800000), both with --leaders=dispatch and --no-smc (neither
modifies its code). Runs from start to end interpret nothing.

What it needed:

- SuperSpider: the long multiplies (UMULL, UMLAL, SMULL, SMLAL: ARMv3M, the
  StrongARM's) decoded, analysed and translated, and in libroger's
  interpreter (expected here, but neither FishTank2 nor DFI's K2 turned
  out to use them); the word after a coprocessor or undefined
  instruction made a dispatch leader (the FPEmulator returns there
  through the dispatcher); gen --no-smc; SP saved in a literal word
  that several routines share - each restores its own (K2's needed it
  more: see its README).
- libroger: SpriteExtend's JPEG SWIs (src/jpeg.c, decoding with
  stb_image); VDU variable 172 (VIDCClockSpeed: 24000kHz on a Risc PC -
  QTM's step table was all zeros without it, and the music only clicks);
  OS_ReadEscapeState seeing a headless script's Escape; --wav taking the
  rate sound is first heard at; the PC histogram sampled rather than
  counted at every instruction (translated code ran twice as fast).

Not done: the 86Hz vsync of the demo's mode (libroger's vsync is always
50Hz); SpriteExtend's transformed JPEG plots.
