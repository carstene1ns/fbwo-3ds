#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <stdbool.h>
#include "structs.h"
#include "level.h"
#include "graphics.h"
#include "audio.h"
#include "config.h"
#include "netconsole.h"

Configuration cfg;

u8 paused = 0; //doesn't necessarily mean that the game is paused
//could be that we're waiting for removal animation to finish
u8 controllable = 1; //ie. during line removal it's set to 0 so stuff won't break;
u8 playable = 1;

u8 mode = MODE_TETRIS;

//controls related variables
u8 start_held = 0;
u8 A_held = 0;
u8 B_held = 0;
u8 HOLD_held = 0;
u8 UP_held = 0;
u8 RIGHT_pressed = 0;
u8 LEFT_pressed = 0;
u32 RIGHT_DAS_count;
u32 LEFT_DAS_count;
u32 RIGHT_DAS_speed_count = 0;
u32 LEFT_DAS_speed_count = 0;

u8 restartpls = 0;

void tetris_control(u32 kDown)
{
    if (kDown & KEY_START)
    {
        if(!start_held)
        {
	    	if(gameover)
	    	{
				playable = 0;
				return;
	    	}
            paused = !paused;
			if(!paused)
				audio_music_play();
            start_held = 1;
        }
    }
    else
        start_held = 0;
    if (kDown & KEY_SELECT)
    {
		if(!gameover && (kDown & KEY_START))
	 	   playable = 0;
		if(gameover && playable)
	    	restartpls = 1;
    }
    if(!paused && controllable && !gameover)
    {
        if(kDown & KEY_A)
        {
            if(!A_held)
            {
                rotate_clockwise();
                A_held = 1;
            }
			if(ARE_state)
			{
				ARE_cw();
			}
        }
        else
            A_held = 0;
        if(kDown & KEY_B)
        {
            if(!B_held)
            {
                rotate_counterclockwise();
                B_held = 1;
            }
	    	if(ARE_state)
	    	{
				ARE_ccw();
	    	}
        }
        else
            B_held = 0;
        if(kDown & cfg.KEY_HOLD && cfg.hold)
        {
            if(!HOLD_held && !cfg.ARS)
            {
				if(!ARE_state)
				     do_hold();
				HOLD_held = 1; 
			}
			else if(ARE_state)
			{
				ARE_hold();
			}
        }
		else
		    HOLD_held = 0;
        if(kDown & KEY_UP)
        {
            if(!UP_held)
            {
                go_all_down();
                UP_held = 1;
            }
        }
        else
            UP_held = 0;
        if(kDown & KEY_DOWN && !ARE_state)
        {
            soft_drop();
        }
        if(kDown & KEY_RIGHT)
        {
            if(!RIGHT_pressed)
            {
				if(!ARE_state)
                    go_right();
                RIGHT_DAS_count--;
                RIGHT_pressed = 1;
            }
            else
            {
                if(RIGHT_DAS_count <= 0) //yes, it can go below 0
                {
                    if(RIGHT_DAS_speed_count == 0 && !ARE_state)
                    {
                        go_right();
                    }
                    if(kDown & cfg.KEY_DAS)
						if(RIGHT_DAS_speed_count > (cfg.DAS_speed >> 1))
			    			RIGHT_DAS_speed_count = 0;
						else
                            RIGHT_DAS_speed_count = (RIGHT_DAS_speed_count + 1) % (cfg.DAS_speed>>1); //boost the DAS!
                    else
                        RIGHT_DAS_speed_count = (RIGHT_DAS_speed_count + 1) % cfg.DAS_speed;
                }
                else
                {
                    if(kDown & cfg.KEY_DAS)
                        RIGHT_DAS_count--; //boost the DAS!
                    RIGHT_DAS_count--;
                } // end RIGHT_DAS_count if
            }//end right pressed if
        }
        else
        {
            //reset values
            RIGHT_DAS_count = cfg.DAS;
            RIGHT_DAS_speed_count = 0;
            RIGHT_pressed = 0;
        }//end right if
        if(kDown & KEY_LEFT)
        {
            if(!LEFT_pressed)
            {
				if(!ARE_state)
                    go_left();
                LEFT_DAS_count--;
                LEFT_pressed = 1;
            }
            else
            {
                if(LEFT_DAS_count <= 0)
                {
                    if(LEFT_DAS_speed_count == 0 && !ARE_state)
                    {
                        go_left();
                    }
                    if(kDown & cfg.KEY_DAS)
						if(LEFT_DAS_speed_count > (cfg.DAS_speed >> 1))
			    			LEFT_DAS_speed_count = 0;
		        		else
                            LEFT_DAS_speed_count = (LEFT_DAS_speed_count + 1) % (cfg.DAS_speed>>1); //boost the DAS!
                    else
                        LEFT_DAS_speed_count = (LEFT_DAS_speed_count + 1) % cfg.DAS_speed;
                }
                else
                {
                    if(kDown & cfg.KEY_DAS)
                        LEFT_DAS_count--; //boost the DAS!
                    LEFT_DAS_count--;
                } // end LEFT_DAS_count if
            }//end LEFT pressed if
        }
        else
        {
            //reset values
            LEFT_DAS_count = cfg.DAS;
            LEFT_DAS_speed_count = 0;
            LEFT_pressed = 0;
        }//end LEFT if
    }
}

int main()
{
    graphics_init();
#ifndef NDEBUG
    netConsoleInit();
#endif
    romfsInit();

    // load config
    default_config();
    printf("reading config...");
    if(!parse_config())
        printf("failed to read config!\ndefault values will be used.\n");

    RIGHT_DAS_count = LEFT_DAS_count = cfg.DAS;

    audio_init(cfg.theme_name); //some of the loading operations will be in another thread so maybe it will be a bit faster

    // load textures
    default_theme();
    if(!load_textures(cfg.theme_name))
		goto texture_error;
    if(!parse_theme(cfg.theme_name))
        printf("failed to read theme config!\ndefault values will be used.\n");
    apply_theme();

    //game init
    init:
    initialize_game();
    paused = 1;
    playable = 1;
    restartpls = 0;
    level = cfg.level;
    load_highscore();
	gameover = 0;

    printf("Game ready! Press START to uh.. start?\n");

    while(aptMainLoop() && playable)
    {
    //controls
        hidScanInput();
        u32 kDown = hidKeysHeld();
        switch(mode)
        {
            case MODE_TETRIS:
                tetris_control(kDown);
				if(!paused  && !gameover)
				{
					audio_music_check();
					if(controllable)
		    			do_gravity();
				}
				else
					audio_music_pause();
                render_frames();
				
		
                break;
            //the following are still in "to-do" - not critical to gameplay
            case MODE_MENU:
		break;
            case MODE_SETTINGS:
		break;
        }
    }
    if(restartpls)
		goto init;
    goto exit;


    texture_error:
    printf("error loading textures! missing files?\n");

    printf("press START to exit.\n");
    while(aptMainLoop())
    {
        hidScanInput();
        u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
    }

    exit:
    save_highscore();
    save_config();
    printf("exiting...\n");
    graphics_fini();
	audio_fini();
    romfsExit();
#ifndef NDEBUG
    netConsoleExit();
#endif

    return 0;
}
