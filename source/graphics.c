#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <citro2d.h>
#include "structs.h"
#include "graphics.h"
#include "level.h"
#include "config.h"

Theme theme;

image background = {0};
image next_text = {0};
image next_frame[6] = {{{0}}};
image grid = {0};
image score_text = {0};
image hiscore_text = {0};
image lines_frame = {0};
image level_frame = {0};
image hold_frame = {0};
image gameover_text = {0};
image paused_text = {0};

image tetris_indicator = {0};
image tspin_indicator = {0};
image tspinsingle_indicator = {0};
image tspindouble_indicator = {0};
image tspintriple_indicator = {0};
image backtoback_indicator = {0};

C2D_Image tetriminos[7];
C2D_Image blocks[7];
C2D_Image score_num[10];
C2D_Image misc_num[10];

Point block_offset_hold;
Point block_offset_next;

s32 remove_line_count;

s32 digit_offset_linesy;
s32 digit_offset_levely;

s32 indicator_frames;
s32 indicator_frame_config;

static C3D_RenderTarget *top_left;
static C3D_RenderTarget *top_right;
static C2D_SpriteSheet spritesheet;

void render_frames() {
	float slider = osGet3DSliderState();

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(top_left, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
	C2D_SceneBegin(top_left);

	if(slider != 0) {
		render_frame(slider * 5.0f);

		C2D_TargetClear(top_right, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
		C2D_SceneBegin(top_right);
		render_frame(slider * -5.0f);
	} else
		render_frame(0);
	C3D_FrameEnd(0);
}

static inline void drawc2dimage_tinted(C2D_Image img, float x, float y, C2D_ImageTint *tint) {
	C2D_DrawImageAt(img, x, y, 0.5f, tint, 1.0f, 1.0f);
}

static inline void drawc2dimage(C2D_Image img, float x, float y) {
	drawc2dimage_tinted(img, x, y, NULL);
}

static inline void drawimageOffXY(image img, int xoffset, int yoffset) {
	drawc2dimage(img.img, img.x + xoffset, img.y + yoffset);
}

static inline void drawimageOffX(image img, int xoffset) {
	drawimageOffXY(img, xoffset, 0);
}

static inline void drawimage(image img) {
	drawimageOffXY(img, 0, 0);
}

void render_grid_blocks(int offset) {
	for(int i = 4; i < DIM_Y; ++i) {
		if(render_line_clear && full_lines[i])
			continue;

		for(int j = 0; j < DIM_X; ++j) {
			int type = level_grid[j][i];
			if(type) {
				drawc2dimage(blocks[type-1],
					grid.x + blocks[type-1].subtex->width * j + offset,
					grid.y + blocks[type-1].subtex->height * (i-4));
			}
		}
	}
}

static void render_counters(int offset) {
	// score
	drawimage(score_text);
	u32 score_temp = score;
	u8 i = 9;
	do { //"do" so it can display '0' too
		s32 digit = score_temp % 10;
		score_temp /= 10;
		drawc2dimage(score_num[digit],
			score_text.x + score_text.w+score_num[digit].subtex->width*i,
			score_text.y);
		i--;
	}
	while(score_temp);

	//high score too
	drawimage(hiscore_text);
	score_temp = high_score;
	i = 9;
	do { //"do" so it can display '0' too
		s32 digit = score_temp % 10;
		score_temp /= 10;
		drawc2dimage(score_num[digit],
			hiscore_text.x + hiscore_text.w+score_num[digit].subtex->width*i,
			hiscore_text.y);
		i--;
	}
	while(score_temp);

	//level
	int level_x = level_frame.x + (level_frame.w>>1);
	int level_y = level_frame.y + digit_offset_levely;
	drawimageOffX(level_frame, offset >> 1);
	if(level >= 10) {
		drawc2dimage(misc_num[level/10],
		          level_x - misc_num[level/10].subtex->width + offset,
		          level_y);
		drawc2dimage(misc_num[level%10],
		          level_x + offset,
		          level_y);
	} else
		drawc2dimage(misc_num[level],
		          level_x - (misc_num[level].subtex->width>>1) + offset,
		          level_y);
	//lines
	int lines_x = lines_frame.x + (lines_frame.w>>1);
	int lines_y = lines_frame.y + digit_offset_linesy;
	drawimageOffX(lines_frame, offset >> 1);
	if(total_lines >= 100) {
		int currentnum = total_lines;
		//last digit
		int w = misc_num[currentnum%10].subtex->width>>1;
		drawc2dimage(misc_num[currentnum%10],
		          lines_x + w + offset,
		          lines_y);
		currentnum = currentnum/10;
		//middle digit
		drawc2dimage(misc_num[currentnum%10],
		          lines_x - w + offset,
		          lines_y);
		currentnum = currentnum/10;
		//first digit (actually this way it won't crash after getting >999, just won't display the number of thousands)
		drawc2dimage(misc_num[currentnum%10],
		          lines_x - 2*w + offset,
		          lines_y);

	} else if(total_lines >= 10) {
		drawc2dimage(misc_num[total_lines/10],
		          lines_x - misc_num[total_lines/10].subtex->width + offset,
		          lines_y);
		drawc2dimage(misc_num[total_lines%10],
		          lines_x + offset,
		          lines_y);
	} else
		drawc2dimage(misc_num[total_lines],
		          lines_x - (misc_num[total_lines].subtex->width>>1) + offset,
		          lines_y);
}

static void render_indicators(int offset) {
	if(indicator_frames >= indicator_frame_config)
		indicator = NONE;

	switch(indicator) {
	case NONE:
		indicator_frames = 0;
		break;
	case TETRIS:
		if(tetris_indicator.valid)
			drawimageOffX(tetris_indicator, offset);
		if(back_to_back_flag_old && backtoback_indicator.valid)
			drawimageOffXY(backtoback_indicator, offset, tetris_indicator.h);
		indicator_frames++;
		break;
	case TSPIN:
		if(tspin_indicator.valid)
			drawimageOffX(tspin_indicator, offset);
		indicator_frames++;
		break;
	case TSPINSINGLE:
		if(tspinsingle_indicator.valid)
			drawimageOffX(tspinsingle_indicator, offset);
		if(back_to_back_flag_old && backtoback_indicator.valid)
			drawimageOffXY(backtoback_indicator, offset, tspinsingle_indicator.h);
		indicator_frames++;
		break;
	case TSPINDOUBLE:
		if(tspindouble_indicator.valid)
			drawimageOffX(tspindouble_indicator, offset);
		if(back_to_back_flag_old && backtoback_indicator.valid)
			drawimageOffXY(backtoback_indicator, offset, tspindouble_indicator.h);
		indicator_frames++;
		break;
	case TSPINTRIPLE:
		if(tspintriple_indicator.valid)
			drawimageOffX(tspintriple_indicator, offset);
		if(back_to_back_flag_old && backtoback_indicator.valid)
			drawimageOffXY(backtoback_indicator, offset, tspintriple_indicator.h);
		indicator_frames++;
		break;
	}
}

void render_frame(int offset) {
	switch(mode) {
	case MODE_TETRIS:
		//stuff that happens always first
		drawimage(background);
		drawimageOffX(grid, offset);

		render_counters(offset);

		if(!paused && !gameover) {
			if(controllable && !ARE_state)
			{ //don't render the block while we're clearing lines nor during ARE.
				if(cfg.ghost_piece)
					render_block(get_ghost_piece(), true, false, offset);

				//display the block
				render_block(*in_play, false, false, offset);
			}

			if(cfg.hold) {
				drawimageOffX(hold_frame, offset >> 1);
				if(hold)
					drawc2dimage(tetriminos[hold->type],
						hold_frame.x + block_offset_hold.x + offset,
						hold_frame.y + block_offset_hold.y);
			}

			if(cfg.next_displayed) {
				Tetrimino_list* element = next_blocks;
				drawimageOffX(next_text, offset >> 1);
				for(int i = 0; i < cfg.next_displayed; ++i) {
					drawimageOffX(next_frame[i], offset >> 1);
					drawc2dimage(tetriminos[element->tetrimino->type],
					          next_frame[i].x + block_offset_next.x + offset,
					          next_frame[i].y + block_offset_next.y);
					element = element->next;
				}
			}

			if(last_deployed) {
				render_block(*last_deployed, false, true, offset);
				free(last_deployed);
				last_deployed = NULL;
			}

			if(!cfg.invisimode) {
				render_grid_blocks(offset);
			}

			if(!render_line_clear)
				remove_line_count = 0;
			else {
				if(remove_line_count == cfg.line_clear_frames) {
					render_line_clear = 0;
					update_level();
					controllable = 1;
				} else {
					controllable = 0;
					//placeholder animation
					u32 clear_color = C2D_Color32(rand() % 255, rand() % 255, rand() % 255, 255);
					for(int i = 4; i < DIM_Y; ++i) {
						if(full_lines[i]) {
							C2D_DrawRectSolid(
								grid.x + offset,
								grid.y + (i-4) * blocks[0].subtex->height,
								0.5f,
								grid.w,
								blocks[0].subtex->height,
								clear_color);
						}
					}
					remove_line_count++;
				}
			}

			render_indicators(offset);

		//end not paused/gameover if
		} else if(paused) {
			drawimageOffX(paused_text, offset);
		} else {
			//must be game over
			render_grid_blocks(offset);
			drawimageOffX(gameover_text, offset);
		}
	}
}

static inline void render_tinted_block(int type, int x, int y, bool ghost_piece, bool lastdepl) {
	u32 c = C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF);
	if(ghost_piece) {
		c = C2D_Color32(0xAA, 0xAA, 0xAA, 0x88);
	} else if(lastdepl) {
		c = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
	}
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint, c, 0.5f);

	drawc2dimage_tinted(blocks[type], x, y, &tint);
}

void render_block(Tetrimino to_render, bool ghost_piece, bool lastdepl, int offset)
{
	int i, j;
	s32 blockw = blocks[to_render.type].subtex->width;
	s32 blockh = blocks[to_render.type].subtex->height;

	if(!cfg.ARS)
	{//SRS
		if(to_render.type != I_TYPE)
		{
			for(i = 0; i < 3; ++i)
				for(j = 0; j < 3; ++j)
				{
					if(rotations[to_render.type][to_render.rotation][j][i])
					{
						if(to_render.pos.y + j < 4)
							continue;
						s32 x = grid.x + blockw * (to_render.pos.x + i) + offset;
						s32 y = grid.y + blockh * (to_render.pos.y - 4 + j);

						render_tinted_block(to_render.type, x, y, ghost_piece, lastdepl);
					}
				}
		}
		else
		{
			for(i = 0; i < 5; ++i)
				for(j = 0; j < 5; ++j)
				{
					if(rotation_I[to_render.rotation][j][i])
					{
						if(to_render.pos.y + j < 4)
							continue;
						s32 x = grid.x + blockh * (to_render.pos.x + i) + offset;
						s32 y = grid.y + blockw * (to_render.pos.y - 4 + j);

						render_tinted_block(to_render.type, x, y, ghost_piece, lastdepl);
					}
				}
		}
	}
	else
	{
		if(to_render.type != I_TYPE)
		{
			for(i = 0; i < 3; ++i)
				for(j = 0; j < 3; ++j)
				{
					if(ARS_rotations[to_render.type][to_render.rotation][j][i])
					{
						if(to_render.pos.y + j < 4)
							continue;
						s32 x = grid.x + blockw * (to_render.pos.x + i) + offset;
						s32 y = grid.y + blockh * (to_render.pos.y - 4 + j);

						render_tinted_block(to_render.type, x, y, ghost_piece, lastdepl);
					}
				}
		}
		else
		{
			for(i = 0; i < 4; ++i)
				for(j = 0; j < 4; ++j)
				{
					if(ARS_rotation_I[to_render.rotation][j][i])
					{
						if(to_render.pos.y + j < 4)
							continue;
						s32 x = grid.x + blockh * (to_render.pos.x + i) + offset;
						s32 y = grid.y + blockw * (to_render.pos.y - 4 + j);

						render_tinted_block(to_render.type, x, y, ghost_piece, lastdepl);
					}
				}
		}
	}
}

/*
Initializes graphics.
*/
void graphics_init()
{
	gfxInitDefault();
	gfxSet3D(1);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	top_left = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	top_right = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);

	consoleInit(GFX_BOTTOM, NULL);
}

void graphics_fini()
{
	C2D_SpriteSheetFree(spritesheet);

	C2D_Fini();
	C3D_Fini();

	gfxExit();
}

static inline image getImageFromSheet(C2D_SpriteSheet sheet, int id) {
	image img;
	img.img = C2D_SpriteSheetGetImage(sheet, id);
	img.w = img.img.subtex->width;
	img.h = img.img.subtex->height;
	img.valid = true;

	return img;
}

/*
Loads necessary textures given the template in "printf" format.
Must include "%s" at the end.
Returns if the load was successful or not.j
*/
bool load_textures(const char* theme_name)
{
	char buffer[80];
	if(!get_theme_file(theme_name, "theme.t3x", buffer)) {
		printf("Textures file not found!\n");
		return false;
	}

	//load spritesheet
	spritesheet = C2D_SpriteSheetLoad(buffer);
	if (!spritesheet) {
		printf("error loading %s\n", buffer);
		return false;
	}
	size_t num_images = C2D_SpriteSheetCount(spritesheet);
	int idx = 0, i;

	background = getImageFromSheet(spritesheet, idx++);
	grid = getImageFromSheet(spritesheet, idx++);
	next_text = getImageFromSheet(spritesheet, idx++);

	for(i = 0; i < 6; ++i)
		next_frame[i] = getImageFromSheet(spritesheet, idx++);

	score_text = getImageFromSheet(spritesheet, idx++);
	hiscore_text = getImageFromSheet(spritesheet, idx++);
	lines_frame = getImageFromSheet(spritesheet, idx++);
	level_frame = getImageFromSheet(spritesheet, idx++);
	hold_frame = getImageFromSheet(spritesheet, idx++);

	gameover_text = getImageFromSheet(spritesheet, idx++);
	paused_text = getImageFromSheet(spritesheet, idx++);

	for(i = 0; i < 7; ++i)
		tetriminos[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);
	for(i = 0; i < 7; ++i)
		blocks[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);
	for (i = 0; i < 10; ++i)
		score_num[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);
	for (i = 0; i < 10; ++i)
		misc_num[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);

	//these are not critical. if they are unavailable, I just won't render them, also for backwards compatibility.
	if (num_images > 50) {
		tetris_indicator = getImageFromSheet(spritesheet, idx++);
		tspin_indicator = getImageFromSheet(spritesheet, idx++);
		tspinsingle_indicator = getImageFromSheet(spritesheet, idx++);
		tspindouble_indicator = getImageFromSheet(spritesheet, idx++);
		tspintriple_indicator = getImageFromSheet(spritesheet, idx++);
		backtoback_indicator = getImageFromSheet(spritesheet, idx++);
	}

	return true;
}

static inline void setImagePos(image *img, Point p) {
	img->x = p.x;
	img->y = p.y;
}

void apply_theme() {
	setImagePos(&background, theme.background);
	setImagePos(&grid, theme.grid);
	setImagePos(&next_text, theme.next_text);
	for(int i = 0; i < 6; ++i)
		setImagePos(&next_frame[i], theme.next_frame[i]);
	setImagePos(&score_text, theme.score_text);
	setImagePos(&hiscore_text, theme.hiscore_text);
	setImagePos(&lines_frame, theme.lines_frame);
	setImagePos(&level_frame, theme.level_frame);
	setImagePos(&hold_frame, theme.hold_frame);

	// just centered
	gameover_text.x = 200 - (gameover_text.w>>1);
	gameover_text.y = 120 - (gameover_text.h>>1);
	paused_text.x = 200 - (paused_text.w>>1);
	paused_text.y = 120 - (paused_text.h>>1);

	block_offset_next.x = theme.offset_next.x;
	block_offset_next.y = theme.offset_next.y;
	block_offset_hold.x = theme.offset_hold.x;
	block_offset_hold.y = theme.offset_hold.y;
	digit_offset_linesy = theme.digit_offset_linesy;
	digit_offset_levely = theme.digit_offset_levely;

	// all on same coords, will be offset manually
	setImagePos(&tetris_indicator, theme.indicators);
	setImagePos(&tspin_indicator, theme.indicators);
	setImagePos(&tspinsingle_indicator, theme.indicators);
	setImagePos(&tspindouble_indicator, theme.indicators);
	setImagePos(&tspintriple_indicator, theme.indicators);
	setImagePos(&backtoback_indicator, theme.indicators);

	indicator_frame_config = theme.indicators_frames;
}
