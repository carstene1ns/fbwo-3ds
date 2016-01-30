#ifndef LEVEL_H
#define LEVEL_H

#include <3ds.h>
#include "structs.h"

int level_grid[DIM_X][DIM_Y]; //24 rows (4 hidden), 10 cols
/*
---------...--->
|0 1 2 3 ... 9
|1
|2
|3
...
|23
v

*/


Tetrimino* in_play;
Tetrimino* hold;
Tetrimino* last_deployed;
Tetrimino_list* next_blocks;

u8 hold_last; //flag to disallow infinite holding
int ticks_before_glue;
u8 gameover;
int score;
int next_counter;
int level;
int total_lines;
int back_to_back_flag;
int render_line_clear;

u32 gravity_frame_counter;

//function declaration
int check_collision(const Tetrimino);
void initialize_game();
int gravity_drop();
void increase_ticks();
void soft_drop();
Tetrimino get_ghost_piece();
int go_all_down();
void do_gravity();
void go_left();
void go_right();
void rotate_clockwise();
void rotate_counterclockwise();
void glue();
void update_level();
void do_gameover();
void recursive_list_cleanup(Tetrimino_list*);
void deploy_next();
void do_hold();
void deploy_hold();
int check_lines();
int check_gameover();
void clear_lines();
void move_down();
Tetrimino_list* generate_bag();
int rand_lim(int);
void print_current_status();
void print_current_state_second_half();
void print_current_state_first_half();

extern Configuration cfg;

u8* full_lines; //array of 24 bytes that tells if any lines are cleared

const int rotation_offsets[2][4][5][2];

//now compliant with The Tetris Company guidelines!
//0-6 are all blocks except I-block, which requires 5x5 matrix, so it's in another variable.
const int rotations[6][4][3][3];
const int rotation_I[4][5][5];

#endif // LEVEL_H












