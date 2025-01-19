#ifndef STRUCTS_H
#define STRUCTS_H

//actually not only structs
typedef enum {
	O_TYPE = 0,
	T_TYPE,
	S_TYPE,
	Z_TYPE,
	J_TYPE,
	L_TYPE,
	I_TYPE
} _type;

#define DIM_X 10
#define DIM_Y 24

#define LINE_FULL 1

typedef enum {
	MODE_TETRIS = 1,
	MODE_MENU,
	MODE_SETTINGS
} _mode;

#if 0
#define dbgprintf(Fmt,...) printf(Fmt, ##__VA_ARGS__)
#else
#define dbgprintf(Fmt,...)
#endif

#include <3ds.h>
#include <citro2d.h>

typedef enum {
	NONE = -1,
	TETRIS,
	TSPIN,
	TSPINSINGLE,
	TSPINDOUBLE,
	TSPINTRIPLE
} Indicator_to_render;

typedef struct _point {
	s32 x;
	s32 y;
} Point;

typedef struct _tetromino {
	Point pos; //top left corner postition
	u8 type;
	u8 rotation;
}Tetrimino;

typedef struct _tetrimino_list {
	Tetrimino* tetrimino;
	struct _tetrimino_list* next;
}Tetrimino_list;

typedef struct _configuration {
	u32 DAS;  //delay (in frames) between pressing the button and moving the block automatically
	u32 DAS_speed; //delay (in frames) between moving the block
	u32 glue_delay[20]; //lock delay - time (in frames) between block making it to the bottom and locking it in place
	bool hold; //hold functionality
	bool ghost_piece; //rendering of the ghost piece
	u8 next_displayed; //no. of next blocks displayed, 0-6
	bool invisimode; //blocks aren't rendered.
	u32 line_clear_frames; //how many frames does the line clear animation last
	u32 frames_per_drop[20]; //how many frames per drop
	u32 rows_per_drop[20]; //how big the drop is
	u32 lines_per_lvl; //how many lines should be cleared before the level is incremented
	bool ARS; //0 for SRS, 1 for ARS (TGM rotation system)
	u32 ARE_delay; //frames between spawning another block
	u8 level; //start level
	u32 KEY_HOLD;
	u32 KEY_DAS;
	char theme_name[32];
} Configuration;
extern Configuration cfg;

typedef struct _theme{
	Point background;
	Point grid;
	Point next_text;
	Point next_frame[6];
	Point score_text;
	Point hiscore_text;
	Point lines_frame;
	Point level_frame;
	Point hold_frame;
	Point offset_next;
	Point offset_hold;
	u32 digit_offset_linesy;
	u32 digit_offset_levely;
	Point indicators;
	u32 indicators_frames;
} Theme;
extern Theme theme;

typedef struct _image {
	C2D_Image img;
	s32 x, y, w, h;
	bool valid;
} image;

// forward declare
typedef struct OggVorbis_File OggVorbis_File;

typedef struct _music{
	u32 chnl; //channel used to play
	u32 channels;
	u32 sample_rate;
	u32 bits_per_sample;
	u32 data_size;
	u32 total_buffer_size;
	u32 last_check;
	u16 bytes_per_sample;
	u16 ndsp_format;
	ndspWaveBuf* first;
	ndspWaveBuf* second;
	u8* first_data;
	u8* second_data;
	OggVorbis_File* file;
} Music;

#endif
