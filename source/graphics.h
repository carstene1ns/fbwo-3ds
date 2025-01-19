#ifndef GRAPHICS_H
#define GRAPHICS_H

void render_grid_blocks(int);
void render_block(Tetrimino, bool, bool, int);
void render_frames();
void render_frame(int);

void graphics_init();
void graphics_fini();

bool load_textures(const char*);
void apply_theme();

#endif // GRAPHICS_H
