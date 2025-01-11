#ifndef LEVEL_H
#define LEVEL_H

#include <3ds.h>
#include "structs.h"

//function declaration
void save_highscore();
void load_highscore();
int check_collision(const Tetrimino);
void initialize_game();
u32 gravity_drop();
void increase_ticks();
void soft_drop();
Tetrimino get_ghost_piece();
int go_all_down();
void do_gravity();
void go_left();
void go_right();
void rotate_clockwise();
void rotate_counterclockwise();
void apply_rotation();
void glue();
void update_level();
void do_gameover();
void iterative_list_cleanup(Tetrimino_list*);
void deploy_next(bool);
void do_hold();
void deploy_hold();
int check_lines();
int check_gameover();
void clear_lines();
void move_down();
Tetrimino_list* generate_bag();
int rand_lim(int);
void ARE_tick();
void ARE_hold();
void ARE_cw();
void ARE_ccw();
void ARE_finish();
u32 T_corners_occupied();

extern int level_grid[DIM_X][DIM_Y];
extern u8 render_line_clear;
extern u8 level;
extern u8 gameover;
extern Tetrimino* last_deployed;
extern u8 back_to_back_flag_old;
extern u8* full_lines;

extern u32 score;
extern u32 high_score;
extern u32 total_lines;
extern u8 paused;
extern u8 controllable;
extern u8 back_to_back_flag;
extern u8 mode;
extern Indicator_to_render indicator;

extern Tetrimino* in_play;
extern Tetrimino* hold;

extern u8 ARE_state;

extern Tetrimino_list* next_blocks;

extern const int rotation_offsets[2][4][5][2];

//now compliant with The Tetris Company guidelines!
//0-6 are all blocks except I-block, which requires 5x5 matrix, so it's in another variable.
extern const int rotations[6][4][3][3];
extern const int rotation_I[4][5][5];

//and now ARS variants
extern const int ARS_rotations[6][4][3][3];
extern const int ARS_rotation_I[4][4][4];

#endif // LEVEL_H
