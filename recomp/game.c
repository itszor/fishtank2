/* FishTank2 (DFI - Paul Thomson, 1997), a Risc PC demo: what libroger
 * needs to know about it.
 *
 * !Run asks for 6700K of application memory, RMLoads QTMTracker (QTM
 * Tracker 1.27, the music player) and runs Fishtank2, an absolute packed
 * with a decruncher of its own (../tools/unpack.py runs it: the image it
 * leaves at &8000 is what's translated). The demo checks it can have 1008K
 * of screen memory (two banks of its mode) and takes it, selects a
 * 448x288 mode at 32 bits per pixel (ModeFile has the monitor definition
 * for it), builds its tables (floating point: the FPEmulator's), plots its
 * JPEG textures with SpriteExtend (JPEG_PlotScaled) to take them back off
 * the screen, starts its music (QTM_Load from memory, QTM_Start) and runs
 * the effects, fading through VIDC20's gamma tables (PaletteV 9).
 *
 * The demo draws as fast as the CPU lets it (it doesn't wait for vsync
 * between frames) and follows the music's position (QTM_Pos) for its
 * sequence, so a faster CPU only means more frames: interactive runs are
 * unthrottled. Escape fades it out and ends it (as at its end: *Desktop).
 *
 * The translated images: the demo (gen/) and QTMTracker (genqtm/, at
 * &1800000). */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <roger.h>

extern const struct roger_image recomp_image, recompQ_image;   /* gen{,qtm}/entries.c */

#define APP "../game/!FishTank2"
static const char *const prefixes[] = {"<Obey$Dir>.", NULL};
static struct roger_module modules[] = {{NULL, &recompQ_image, 0, NULL, 0}, {NULL, NULL, 0, NULL, 0}};

/* a file in !FishTank2, whatever ,load-exec suffix the extraction gave it */
static const char *in_app(const char *leaf)
{
    DIR *d = opendir(APP);
    struct dirent *e;
    char *path = NULL;
    while (d && !path && (e = readdir(d)))
        if (strcspn(e->d_name, ",") == strlen(leaf) && !strncasecmp(e->d_name, leaf, strlen(leaf))) {
            path = malloc(strlen(APP) + strlen(e->d_name) + 2);
            sprintf(path, "%s/%s", APP, e->d_name);
        }
    if (d) closedir(d);
    if (!path) { fprintf(stderr, "no %s/%s: build.sh extracts it\n", APP, leaf); exit(1); }
    return path;
}

static struct roger_config config = {
    .title = "FishTank2 (recompiled)",
    .image = &recomp_image,
    .modules = modules,
    .appdir = "../game",
    .dir_prefixes = prefixes,
    .vidc20 = 1,                        /* a Risc PC: 32bpp modes, mode selectors, gamma */
    .initial_mode = 28,                 /* a desktop's: 640x480, 256 colours */
    .wimpslot = 6700 * 1024,            /* !Run: wimpslot -min 6700k */
    /* headless runs (--limit) at a StrongARM's speed, which it was written
     * for: as many frames against the music as on a Risc PC (its tables take
     * 480 million instructions; it plays for some 54 billion) */
    .headless_mips = 200,
    /* code the analysis missed (none seen in runs from start to end) is
     * interpreted rather than ending the run */
    .interpret_gaps = 1,
};

int main(int argc, char **argv)
{
    modules[0].path = in_app("QTMTracker");
    config.image_path = "../unpacked/fishtank2";
    roger_main(argc, argv, &config);
}
