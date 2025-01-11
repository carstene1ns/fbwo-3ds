#include "graphics.h"
#include <stdio.h>
#include <stdbool.h>

static const int DEFAULT_NEXT_FRAME_X[] = { 260, 295, 330, 365, 365, 365};
static const int DEFAULT_NEXT_FRAME_Y[] = { 20, 20, 20, 20, 60, 100 };

image background;
image next_text;
image next_frame[6];
image grid;
image score_text;
image hiscore_text;
image lines_frame;
image level_frame;
image hold_frame;

C2D_Image tetriminos[7];
C2D_Image blocks[7];
C2D_Image score_num[10];
C2D_Image misc_num[10];
C2D_Image gameover_text;
C2D_Image paused_text;

C2D_Image tetris_indicator;
C2D_Image tspin_indicator;
C2D_Image tspinsingle_indicator;
C2D_Image tspindouble_indicator;
C2D_Image tspintriple_indicator;
C2D_Image backtoback_indicator;

s32 block_offset_holdx;
s32 block_offset_holdy;

s32 block_offset_nextx;
s32 block_offset_nexty;

s32 remove_line_count;

s32 digit_offset_linesy;
s32 digit_offset_levely;

s32 indicatorx;
s32 indicatory;

s32 indicator_frames;
s32 indicator_frame_config;

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

static C3D_RenderTarget *top_left;
static C3D_RenderTarget *top_right;
static C2D_SpriteSheet spritesheet;

void render_frames()
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(top_left, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
	C2D_SceneBegin(top_left);

	if(CONFIG_3D_SLIDERSTATE != 0)
	{
		render_frame(CONFIG_3D_SLIDERSTATE * 5.0f);

		C2D_TargetClear(top_right, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
		C2D_SceneBegin(top_right);
		render_frame(CONFIG_3D_SLIDERSTATE * -5.0f);
	} else
		render_frame(0);
	C3D_FrameEnd(0);
}

void drawimage_tinted(C2D_Image img, float x, float y, C2D_ImageTint *tint)
{
	C2D_DrawImageAt(img, x, y, 0.5f, tint, 1.0f, 1.0f);
}

void drawimage(C2D_Image img, float x, float y)
{
	C2D_DrawImageAt(img, x, y, 0.5f, NULL, 1.0f, 1.0f);
}

void render_grid_blocks(int offset)
{
	for(int i = 4; i < DIM_Y; ++i)
	{
	    if(render_line_clear && full_lines[i])
		continue;
	    for(int j = 0; j < DIM_X; ++j)
	    {
		int type;
		if((type = level_grid[j][i]))
		    drawimage(blocks[type-1], grid.posx + blocks[type-1].subtex->width * j + offset, grid.posy + blocks[type-1].subtex->height * (i-4));
	    }
	}
}

void render_frame(int offset)
{     
        switch(mode)
        {
            case MODE_TETRIS:
                //stuff that happens always first
                drawimage(background.img, background.posx, background.posy);
                drawimage(grid.img, grid.posx + offset, grid.posy);
                drawimage(score_text.img, score_text.posx, score_text.posy);
                u32 score_temp = score;
                u8 i = 9;
                do{ //"do" so it can display '0' too
                    s32 digit = score_temp % 10;
                    score_temp /= 10;
                    drawimage(score_num[digit], score_text.posx + score_text.img.subtex->width+score_num[digit].subtex->width*i, score_text.posy);
                    i--;
                }
                while(score_temp);
		//high score too
                drawimage(hiscore_text.img, hiscore_text.posx, hiscore_text.posy);
                score_temp = high_score;
                i = 9;
                do{ //"do" so it can display '0' too
                    s32 digit = score_temp % 10;
                    score_temp /= 10;
                    drawimage(score_num[digit], hiscore_text.posx + hiscore_text.img.subtex->width+score_num[digit].subtex->width*i, hiscore_text.posy);
                    i--;
                }
                while(score_temp);

                //level
                drawimage(level_frame.img, level_frame.posx + (offset >> 1), level_frame.posy);
                if(level >= 10)
                {
                    drawimage(
                              misc_num[level/10],
                              level_frame.posx + (level_frame.img.subtex->width>>1) - misc_num[level/10].subtex->width + offset,
                              level_frame.posy + digit_offset_levely
                              );
                    drawimage(
                              misc_num[level%10],
                              level_frame.posx + (level_frame.img.subtex->width>>1) + offset,
                              level_frame.posy + digit_offset_levely
                              );
                }
                else
                    drawimage(
                              misc_num[level],
                              level_frame.posx + ((level_frame.img.subtex->width>>1) - (misc_num[level].subtex->width>>1)) + offset,
                              level_frame.posy + digit_offset_levely
                              );
                //lines
                drawimage(lines_frame.img, lines_frame.posx + (offset >> 1), lines_frame.posy);
                if(total_lines >= 100)
                {
		    int currentnum = total_lines;
		//last digit
                    drawimage(
                              misc_num[currentnum%10],
                              lines_frame.posx + (lines_frame.img.subtex->width>>1) + (misc_num[currentnum%10].subtex->width>>1) + offset,
                              lines_frame.posy + digit_offset_linesy
                              );
		    currentnum = currentnum/10;
		//middle digit
                    drawimage(
                              misc_num[currentnum%10],
                              lines_frame.posx + (lines_frame.img.subtex->width>>1) - (misc_num[currentnum%10].subtex->width>>1) + offset,
                              lines_frame.posy + digit_offset_linesy
                              );
		    currentnum = currentnum/10;
		//first digit (actually this way it won't crash after getting >999, just won't display the number of thousands)
                    drawimage(
                              misc_num[currentnum%10],
                              lines_frame.posx + (lines_frame.img.subtex->width>>1) - (misc_num[currentnum%10].subtex->width>>1) - misc_num[currentnum%10].subtex->width + offset,
                              lines_frame.posy + digit_offset_linesy
                              );
                    
                }
                else if(total_lines >= 10)
                {
                    drawimage(
                              misc_num[total_lines/10],
                              lines_frame.posx + (lines_frame.img.subtex->width>>1) - misc_num[total_lines/10].subtex->width + offset,
                              lines_frame.posy + digit_offset_linesy
                              );
                    drawimage(
                              misc_num[total_lines%10],
                              lines_frame.posx + (lines_frame.img.subtex->width>>1) + offset,
                              lines_frame.posy + digit_offset_linesy
                              );
                }
                else
                    drawimage(
                              misc_num[total_lines],
                              lines_frame.posx + ((lines_frame.img.subtex->width>>1) - (misc_num[total_lines].subtex->width>>1)) + offset,
                              lines_frame.posy + digit_offset_linesy
                              );
                if(!paused && !gameover)
                {
		    if(controllable && !ARE_state)
		    { //don't render the block while we're clearing lines nor during ARE.
		            if(cfg.ghost_piece)
		                render_block(get_ghost_piece(), true, false, offset);

		            //display the block
		            render_block(*in_play, false, false, offset);
		    }
                    if(cfg.hold)
                    {
                        drawimage(hold_frame.img, hold_frame.posx + (offset >> 1), hold_frame.posy);
			if(hold)
                        drawimage(tetriminos[hold->type], hold_frame.posx + block_offset_holdx + offset, hold_frame.posy + block_offset_holdy);
                    }
                    if(cfg.next_displayed)
                    {
                        drawimage(next_frame[0].img, next_frame[0].posx + (offset >> 1), next_frame[0].posy);
                        drawimage(next_text.img, next_text.posx + (offset >> 1), next_text.posy);
                        drawimage(
                                  tetriminos[next_blocks->tetrimino->type],
                                  next_frame[0].posx + block_offset_nextx + offset,
                                  next_frame[0].posy + block_offset_nexty
                                  );
                        Tetrimino_list* element = next_blocks;
                        for(int i = 1; i < cfg.next_displayed; ++i)
                        {
                            element = element->next;
                            drawimage(next_frame[i].img, next_frame[i].posx + (offset >> 1), next_frame[i].posy);
                            drawimage(
                                      tetriminos[element->tetrimino->type],
                                      next_frame[i].posx + block_offset_nextx + offset,
                                      next_frame[i].posy + block_offset_nexty
                                      );
                        }
                    }

                    if(last_deployed)
                    {
                        render_block(*last_deployed, false, true, offset);
                        free(last_deployed);
                        last_deployed = NULL;
                    }
                    if(!cfg.invisimode)
                    {
			render_grid_blocks(offset);
                    }
                    if(!render_line_clear)
                        remove_line_count = 0;
                    else
                    {
                        if(remove_line_count == cfg.line_clear_frames)
                        {
                            render_line_clear = 0;
                            update_level();
                            controllable = 1;
                        }
                        else
                        {
                            controllable = 0;
                        //placeholder animation
                            u32 clear_color = C2D_Color32(rand() % 255, rand() % 255, rand() % 255, rand() % 255);
                            for(int i = 4; i < DIM_Y; ++i)
                            {
                                if(full_lines[i])
                                {
                                    C2D_DrawRectSolid(
                                        grid.posx + offset,
                                        grid.posy + (i-4) * blocks[0].subtex->height,
                                        0.5f,
                                        grid.img.subtex->width,
                                        blocks[0].subtex->height,
                                        clear_color
                                    );
                                }
                            }
                            remove_line_count++;
                        }
                    }
		    if(indicator_frames >= indicator_frame_config)
			indicator = NONE;
		    switch(indicator)
		    {
			case NONE:
			    indicator_frames = 0;
			    break;
			case TETRIS:
			    if(tetris_indicator.tex)
				drawimage(
					   	  tetris_indicator,
						  indicatorx + offset,
						  indicatory
						 );
			    if(back_to_back_flag_old && backtoback_indicator.tex)
				drawimage(
					   	  backtoback_indicator,
						  indicatorx + offset,
						  indicatory + tetris_indicator.subtex->height
						 );
			    indicator_frames++;
			    break;
			case TSPIN:
			    if(tspin_indicator.tex)
				drawimage(
					   	  tspin_indicator,
						  indicatorx + offset,
						  indicatory
						 );
			    indicator_frames++;
			    break;
			case TSPINSINGLE:
			    if(tspinsingle_indicator.tex)
				drawimage(
					   	  tspinsingle_indicator,
						  indicatorx + offset,
						  indicatory
						 );
			    if(back_to_back_flag_old && backtoback_indicator.tex)
				drawimage(
					   	  backtoback_indicator,
						  indicatorx + offset,
						  indicatory + tspinsingle_indicator.subtex->height
						 );
			    indicator_frames++;
			    break;
			case TSPINDOUBLE:
			    if(tspindouble_indicator.tex)
				drawimage(
					   	  tspindouble_indicator,
						  indicatorx + offset,
						  indicatory
						 );
			    if(back_to_back_flag_old && backtoback_indicator.tex)
				drawimage(
					   	  backtoback_indicator,
						  indicatorx + offset,
						  indicatory + tspindouble_indicator.subtex->height
						 );
			    indicator_frames++;
			    break;
			case TSPINTRIPLE:
			    if(tspintriple_indicator.tex)
				drawimage(
					   	  tspintriple_indicator,
						  indicatorx + offset,
						  indicatory
						 );
			    if(back_to_back_flag_old && backtoback_indicator.tex)
				drawimage(
					   	  backtoback_indicator,
						  indicatorx + offset,
						  indicatory + tspintriple_indicator.subtex->height
						 );
			    indicator_frames++;
			    break;

		    }
                }//end not paused/gameover if
                else if(paused)
                {
                    drawimage(paused_text, 200 - (paused_text.subtex->width>>1) + offset, 120 - (paused_text.subtex->height>>1));
                }
                else //must be game over
                {
		    render_grid_blocks(offset);
                    drawimage(gameover_text, 200 - (gameover_text.subtex->width>>1) + offset, 120 - (gameover_text.subtex->height>>1));

                }


        }
}

/*
Loads necessary textures given the template in "printf" format.
Must include "%s" at the end.
Returns if the load was successful or not.j
*/
int load_textures(const char* str_template)
{
	char buffer[80];

	//load spritesheet
	sprintf(buffer, str_template, "theme.t3x");
	spritesheet = C2D_SpriteSheetLoad(buffer);
	if (!spritesheet) {
		printf("error loading %s\n", buffer);
		return 0;
	}
	size_t num_images = C2D_SpriteSheetCount(spritesheet);
	int idx = 0, i;

	background.posx = 0;
	background.posy = 0;
	background.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	grid.posx = DEFAULT_GRIDX;
	grid.posy = DEFAULT_GRIDY;
	grid.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	next_text.posx = DEFAULT_NEXT_TEXTX;
	next_text.posy = DEFAULT_NEXT_TEXTY;
	next_text.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	for(i = 0; i < 6; ++i)
	{
		next_frame[i].posx = DEFAULT_NEXT_FRAME_X[i];
		next_frame[i].posy = DEFAULT_NEXT_FRAME_Y[i];
		next_frame[i].img = C2D_SpriteSheetGetImage(spritesheet, idx++);
	}

	score_text.posx = DEFAULT_SCORE_TEXTX;
	score_text.posy = DEFAULT_SCORE_TEXTY;
	score_text.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	hiscore_text.posx = DEFAULT_HISCORE_TEXTX;
	hiscore_text.posy = DEFAULT_HISCORE_TEXTY;
	hiscore_text.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	lines_frame.posx = DEFAULT_LINES_FRAMEX;
	lines_frame.posy = DEFAULT_LINES_FRAMEY;
	lines_frame.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	level_frame.posx = DEFAULT_LEVEL_FRAMEX;
	level_frame.posy = DEFAULT_LEVEL_FRAMEY;
	level_frame.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	hold_frame.posx = DEFAULT_HOLD_FRAMEX;
	hold_frame.posy = DEFAULT_HOLD_FRAMEY;
	hold_frame.img = C2D_SpriteSheetGetImage(spritesheet, idx++);

	gameover_text = C2D_SpriteSheetGetImage(spritesheet, idx++);
	paused_text = C2D_SpriteSheetGetImage(spritesheet, idx++);

	for(i = 0; i < 7; ++i)
		tetriminos[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);
	for(i = 0; i < 7; ++i)
		blocks[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);
	for (i = 0; i < 10; ++i)
		score_num[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);
	for (i = 0; i < 10; ++i)
		misc_num[i] = C2D_SpriteSheetGetImage(spritesheet, idx++);

	//these are not critical. if they are unavailable, I just won't render them, also for backwards compatibility.
	if (num_images > 50)
	{
		tetris_indicator = C2D_SpriteSheetGetImage(spritesheet, idx++);
		tspin_indicator = C2D_SpriteSheetGetImage(spritesheet, idx++);
		tspinsingle_indicator = C2D_SpriteSheetGetImage(spritesheet, idx++);
		tspindouble_indicator = C2D_SpriteSheetGetImage(spritesheet, idx++);
		tspintriple_indicator = C2D_SpriteSheetGetImage(spritesheet, idx++);
		backtoback_indicator = C2D_SpriteSheetGetImage(spritesheet, idx++);
	}
	block_offset_holdx = DEFAULT_OFFSET_HOLDX;
	block_offset_holdy = DEFAULT_OFFSET_HOLDY;
	block_offset_nextx = DEFAULT_OFFSET_NEXTX;
	block_offset_nexty = DEFAULT_OFFSET_NEXTY;
	digit_offset_linesy = DEFAULT_OFFSET_LNSY;
	digit_offset_levely = DEFAULT_OFFSET_LVLY;
	indicatorx = DEFAULT_POPUPX;
	indicatory = DEFAULT_POPUPY;
	indicator_frame_config = DEFAULT_POPUP_FRAME_CFG;

	return 1;
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

/*
Parses theme.cfg.
*/
void graphics_parse_config(char* theme_template)
{
    char buffer[80];
    char command[32];
    s32 valx, valy;

    sprintf(buffer, theme_template, "theme.cfg");
    FILE* cfgf = fopen(buffer, "r");
    if(!cfgf)
    {
	printf("error opening %s", buffer);
	return;
    }
    while(fgets(buffer, 79, cfgf))
    {
        
        if(sscanf(buffer, "%s %ld %ld", command, &valx, &valy) == 3)
        {
            if(!strcmp(command, "grid"))
            {
                dbgprintf("grid = [%ld, %ld]\n", valx, valy);
                grid.posx = valx;
                grid.posy = valy;
            }

            else if(!strcmp(command, "next_text"))
            {
                dbgprintf("next_text = [%ld, %ld]\n", valx, valy);
                next_text.posx = valx;
                next_text.posy = valy;
            }
            else if(!strcmp(command, "next_frame0"))
            {
                dbgprintf("next_frame0 = [%ld, %ld]\n", valx, valy);
                next_frame[0].posx = valx;
                next_frame[0].posy = valy;
            }
            else if(!strcmp(command, "next_frame1"))
            {
                dbgprintf("next_frame1 = [%ld, %ld]\n", valx, valy);
                next_frame[1].posx = valx;
                next_frame[1].posy = valy;
            }
            else if(!strcmp(command, "next_frame2"))
            {
                dbgprintf("next_frame2 = [%ld, %ld]\n", valx, valy);
                next_frame[2].posx = valx;
                next_frame[2].posy = valy;
            }
            else if(!strcmp(command, "next_frame3"))
            {
                dbgprintf("next_frame3 = [%ld, %ld]\n", valx, valy);
                next_frame[3].posx = valx;
                next_frame[3].posy = valy;
            }
            else if(!strcmp(command, "next_frame4"))
            {
                dbgprintf("next_frame4 = [%ld, %ld]\n", valx, valy);
                next_frame[4].posx = valx;
                next_frame[4].posy = valy;
            }
            else if(!strcmp(command, "next_frame5"))
            {
                dbgprintf("next_frame5 = [%ld, %ld]\n", valx, valy);
                next_frame[5].posx = valx;
                next_frame[5].posy = valy;
            }
            else if(!strcmp(command, "score_text"))
            {
                dbgprintf("score_text = [%ld, %ld]\n", valx, valy);
                score_text.posx = valx;
                score_text.posy = valy;
            }
            else if(!strcmp(command, "hiscore_text"))
            {
                dbgprintf("hiscore_text = [%ld, %ld]\n", valx, valy);
                hiscore_text.posx = valx;
                hiscore_text.posy = valy;
            }
            else if(!strcmp(command, "lines_frame"))
            {
                dbgprintf("lines_frame = [%ld, %ld]\n", valx, valy);
                lines_frame.posx = valx;
                lines_frame.posy = valy;
            }
            else if(!strcmp(command, "level_frame"))
            {
                dbgprintf("level_frame = [%ld, %ld]\n", valx, valy);
                level_frame.posx = valx;
                level_frame.posy = valy;
            }
            else if(!strcmp(command, "hold_frame"))
            {
                dbgprintf("hold_frame = [%ld, %ld]\n", valx, valy);
                hold_frame.posx = valx;
                hold_frame.posy = valy;
            }
            else if(!strcmp(command, "offset_next"))
            {
                dbgprintf("offset_next = [%ld, %ld]\n", valx, valy);
                block_offset_nextx = valx;
                block_offset_nexty = valy;
            }
            else if(!strcmp(command, "offset_hold"))
            {
                dbgprintf("offset_hold = [%ld, %ld]\n", valx, valy);
                block_offset_holdx = valx;
                block_offset_holdy = valy;
            }
            else if(!strcmp(command, "indicators"))
            {
                dbgprintf("indicators = [%ld, %ld]\n", valx, valy);
                indicatorx = valx;
                indicatory = valy;
            }
        }
        else if(sscanf(buffer, "%s %ld", command, &valx) == 2)
        {
	    if(!strcmp(command, "lines_dig_off"))
            {
                dbgprintf("lines digit offset = %ld\n", valx);
                digit_offset_linesy = valx;
            }
	    else if(!strcmp(command, "level_dig_off"))
            {
                dbgprintf("level digit offset = %ld\n", valx);
                digit_offset_levely = valx;
            }
	    else if(!strcmp(command, "indicator_frames"))
            {
                dbgprintf("indicator frame time = %ld\n", valx);
                indicator_frame_config = valx;
            }
	}
    }
    fclose(cfgf);
}

void render_block(Tetrimino to_render, bool ghost_piece, bool lastdepl, int offset)
{
	C2D_ImageTint tint;

    if(!cfg.ARS)
    {//SRS
	    if(to_render.type != I_TYPE)
	    {
		int i, j;
		for(i = 0; i < 3; ++i)
		    for(j = 0; j < 3; ++j)
		    {
		        if(rotations[to_render.type][to_render.rotation][j][i])
		        {
			    if(to_render.posy + j < 4)
				continue;
                            s32 x = grid.posx + blocks[to_render.type].subtex->width * (to_render.posx + i) + offset;
                            s32 y = grid.posy + blocks[to_render.type].subtex->height * (to_render.posy - 4 + j);
                            if(ghost_piece) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xAA, 0xAA, 0xAA, 0x88), 0.5f);
                                drawimage_tinted(blocks[to_render.type], x, y, &tint);
                            } else if(lastdepl) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF), 0.5f);
                                drawimage_tinted(blocks[to_render.type], x, y, &tint);
                            } else {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF), 0.5f);
                                drawimage_tinted(blocks[to_render.type], x, y, &tint);
                            }
		        }
		    }
	    }
	    else
	    {
		int i, j;
		for(i = 0; i < 5; ++i)
		    for(j = 0; j < 5; ++j)
		    {
		        if(rotation_I[to_render.rotation][j][i])
		        {
			    if(to_render.posy + j < 4)
				continue;
                            s32 x = grid.posx + blocks[I_TYPE].subtex->height * (to_render.posx + i) + offset;
                            s32 y = grid.posy + blocks[I_TYPE].subtex->width * (to_render.posy - 4 + j);
                            if(ghost_piece) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xAA, 0xAA, 0xAA, 0x88), 0.5f);
                                drawimage_tinted(blocks[I_TYPE], x, y, &tint);
                            } else if(lastdepl) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF), 0.5f);
                                drawimage_tinted(blocks[I_TYPE], x, y, &tint);
                            } else {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF), 0.5f);
                                drawimage_tinted(blocks[I_TYPE], x, y, &tint);
                            }
		        }
		    }
	    }
    }
    else
    {
		    if(to_render.type != I_TYPE)
	    {
		int i, j;
		for(i = 0; i < 3; ++i)
		    for(j = 0; j < 3; ++j)
		    {
		        if(ARS_rotations[to_render.type][to_render.rotation][j][i])
		        {
			    if(to_render.posy + j < 4)
				continue;
                            s32 x = grid.posx + blocks[to_render.type].subtex->width * (to_render.posx + i) + offset;
                            s32 y = grid.posy + blocks[to_render.type].subtex->height * (to_render.posy - 4 + j);
                            if(ghost_piece) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xAA, 0xAA, 0xAA, 0x88), 0.5f);
                                drawimage_tinted(blocks[to_render.type], x, y, &tint);
                            } else if(lastdepl) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF), 0.5f);
                                drawimage_tinted(blocks[to_render.type], x, y, &tint);
                            } else {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF), 0.5f);
                                drawimage_tinted(blocks[to_render.type], x, y, &tint);
                            }
		        }
		    }
	    }
	    else
	    {
		int i, j;
		for(i = 0; i < 4; ++i)
		    for(j = 0; j < 4; ++j)
		    {
		        if(ARS_rotation_I[to_render.rotation][j][i])
		        {
			    if(to_render.posy + j < 4)
				continue;
                            s32 x = grid.posx + blocks[I_TYPE].subtex->height * (to_render.posx + i) + offset;
                            s32 y = grid.posy + blocks[I_TYPE].subtex->width * (to_render.posy - 4 + j);
                            if(ghost_piece) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xAA, 0xAA, 0xAA, 0x88), 0.5f);
                                drawimage_tinted(blocks[I_TYPE], x, y, &tint);
                            } else if(lastdepl) {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF), 0.5f);
                                drawimage_tinted(blocks[I_TYPE], x, y, &tint);
                            } else {
                                C2D_PlainImageTint(&tint, C2D_Color32(0xDD, 0xDD, 0xDD, 0xFF), 0.5f);
                                drawimage_tinted(blocks[I_TYPE], x, y, &tint);
                            }
		        }
		    }
	    }
    }
}
void graphics_fini()
{
	C2D_SpriteSheetFree(spritesheet);

	C2D_Fini();
	C3D_Fini();
	gfxExit();
}
