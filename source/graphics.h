#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "level.h"
#include "structs.h"
#include <citro2d.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_GRIDX           150
#define DEFAULT_GRIDY           20

#define DEFAULT_NEXT_TEXTX      260
#define DEFAULT_NEXT_TEXTY      55

#define DEFAULT_SCORE_TEXTX     280
#define DEFAULT_SCORE_TEXTY     210

#define DEFAULT_HISCORE_TEXTX     0
#define DEFAULT_HISCORE_TEXTY     210

#define DEFAULT_LINES_FRAMEX    115
#define DEFAULT_LINES_FRAMEY    140

#define DEFAULT_LEVEL_FRAMEX    115
#define DEFAULT_LEVEL_FRAMEY    190

#define DEFAULT_HOLD_FRAMEX     115
#define DEFAULT_HOLD_FRAMEY     20

#define DEFAULT_OFFSET_HOLDX    5
#define DEFAULT_OFFSET_HOLDY    5

#define DEFAULT_OFFSET_NEXTX    5
#define DEFAULT_OFFSET_NEXTY    5

#define DEFAULT_OFFSET_LVLY	10
#define DEFAULT_OFFSET_LNSY	10

#define DEFAULT_POPUPX		270
#define DEFAULT_POPUPY		180

#define DEFAULT_POPUP_FRAME_CFG	40

void render_grid_blocks(int);
void render_block(Tetrimino, bool, bool, int);
void render_frames();
void render_frame(int);
void graphics_parse_config(char*);
void graphics_fini();
void graphics_init();
int load_textures(const char*);

#endif // GRAPHICS_H
