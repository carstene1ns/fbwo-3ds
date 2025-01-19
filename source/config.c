#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "structs.h"
#include "config.h"

#define INI_HANDLER_LINENO 1
#define INI_ALLOW_MULTILINE 0
#define INI_ALLOW_BOM 0
#include "ext/ini.h"

/*
init config w/ def. values
*/
void default_config() {
	cfg.DAS = 11;
	cfg.DAS_speed = 6;
	for(int i = 0; i < 20; ++i)
		cfg.glue_delay[i] = 30;
	cfg.hold = true;
	cfg.ghost_piece = true;
	cfg.next_displayed = 5;
	cfg.invisimode = false;
	cfg.line_clear_frames = 40;
	// level:          1   2   3   4   5   6   7   8  9  10 11 12 13 14 15 16 17 18 19 20 
	const u32 fpd[] = {30, 28, 27, 24, 20, 15, 10, 8, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	memcpy(cfg.frames_per_drop, fpd, sizeof(u32)*20); 
	// level:          1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17  18  19  20
	const u32 rpd[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 6, 8, 10, 12, 15, 20};
	memcpy(cfg.rows_per_drop, rpd, sizeof(u32)*20); 
	cfg.lines_per_lvl = 10;
	cfg.ARS = false;
	cfg.ARE_delay = 0;
	cfg.level = 1;
	cfg.KEY_HOLD = KEY_L;
	cfg.KEY_DAS = KEY_R;
	sprintf(cfg.theme_name, "default");
}

/*
init theme w/ def. values
*/
void default_theme() {
	#define setXY(v,vx,vy) \
		do { \
			v.x = (vx); \
			v.y = (vy); \
		} while(0)

	setXY(theme.background, 0, 0);
	setXY(theme.grid, 150, 20);
	setXY(theme.next_text, 260, 55);

	const int DEFAULT_NEXT_FRAME_X[] = { 260, 295, 330, 365, 365, 365};
	const int DEFAULT_NEXT_FRAME_Y[] = { 20, 20, 20, 20, 60, 100 };
	for(int i = 0; i < 6; ++i)
		setXY(theme.next_frame[i], DEFAULT_NEXT_FRAME_X[i], DEFAULT_NEXT_FRAME_Y[i]);

	setXY(theme.score_text, 280, 210);
	setXY(theme.hiscore_text, 0, 210);
	setXY(theme.lines_frame, 115, 140);
	setXY(theme.level_frame, 115, 190);
	setXY(theme.hold_frame, 115, 20);
	setXY(theme.offset_hold, 5, 5);
	setXY(theme.offset_next, 5, 5);
	theme.digit_offset_linesy = 10;
	theme.digit_offset_levely = 10;
	setXY(theme.indicators, 270, 180);
	theme.indicators_frames = 40;

	#undef setXY
}

static int config_handler(void* user, const char* section,
                          const char* name, const char* value, int lineno) {

	bool ret = 1;

	#define MATCH_S(s) strcmp(section, s) == 0
	#define MATCH_N(n) strcmp(name, n) == 0
	#define MATCH_BOOL(var) \
		do { \
			if((strlen(value) == 1 && value[0] == '1') \
				|| (strlen(value) == 2 && ((value[0] == 'o' || value[0] == 'O') || \
				(value[1] == 'n' ||value[1] == 'N')))) { \
				var = true; \
			} else { \
				var = false; \
			} \
		} while(0)

	if(MATCH_S("game")) {
		// game
		if(MATCH_N("DAS_delay")) {
			int dly = strtol(value, NULL, 10);
			if(dly >= 0)
				cfg.DAS = dly;
		} else if(MATCH_N("DAS_speed")) {
			int spd = strtol(value, NULL, 10);
			if(spd >= 0)
				cfg.DAS_speed = spd;
		} else if(MATCH_N("invisimode")) {
			MATCH_BOOL(cfg.invisimode);
		} else if(MATCH_N("hold")) {
			MATCH_BOOL(cfg.hold);
		} else if(MATCH_N("hold_key")) {
			if(strlen(value) == 1 &&
				(value[0] == 'l' || value[0] == 'L')) {
				cfg.KEY_HOLD = KEY_L;
				cfg.KEY_DAS = KEY_R;
			} else {
				cfg.KEY_HOLD = KEY_R;
				cfg.KEY_DAS = KEY_L;
			}
		} else if(MATCH_N("rotation_system")) {
			if(strlen(value) == 3 &&
				(value[0] == 'a' || value[0] == 'A') &&
				(value[1] == 'r' || value[1] == 'R') &&
				(value[2] == 's' || value[2] == 'S')) {
				cfg.ARS = 1;
			} else {
				cfg.ARS = 0;
			}
		} else if(MATCH_N("ghost_piece")) {
			MATCH_BOOL(cfg.ghost_piece);
		} else if(MATCH_N("ARE_delay")) {
			int dly = strtol(value, NULL, 10);
			if(dly >= 0)
				cfg.ARE_delay = dly;
		} else if(MATCH_N("line_clear_frames")) {
			int frms = strtol(value, NULL, 10);
			if(frms >= 0)
				cfg.line_clear_frames = frms;
		} else if(MATCH_N("lines_per_level")) {
			int lns = strtol(value, NULL, 10);
			if(lns >= 0)
				cfg.lines_per_lvl = lns;
		} else if(MATCH_N("next_displayed")) {
			int nd = strtol(value, NULL, 10);
			if(nd >= 0 && nd < 7)
				cfg.next_displayed = nd;
		} else if(MATCH_N("level")) {
			int lvl = strtol(value, NULL, 10);
			if(lvl > 0 && lvl <= 20)
				cfg.level = lvl;
		} else
			ret = 0; // unknown
	} else if(MATCH_S("theme")) {
		// theme
		if(MATCH_N("path")) {
			if(strcmp("default", value) != 0) {
				size_t sz = sizeof(cfg.theme_name);
				snprintf(cfg.theme_name, sz, "%s", value);
				cfg.theme_name[sz-1] = '\0';
			}
		} else
			ret = 0; // unknown
	} else if(strncmp(section, "level", 5) == 0) {
		// levels
		char key[10];
		bool found = false;
		for(int i = 0; i < 20; i++) {
			sprintf(key, "level %d", i+1);
			if(MATCH_S(key)) {
				if(MATCH_N("frames_per_drop")) {
					int frms = strtol(value, NULL, 10);
					if(frms >= 0)
						cfg.frames_per_drop[i] = frms;
					found = true;
					break;
				} else if(MATCH_N("rows_per_drop")) {
					int rws = strtol(value, NULL, 10);
					if(rws >= 0)
						cfg.rows_per_drop[i] = rws;
					found = true;
					break;
				} else if(MATCH_N("glue_delay")) {
					int gd = strtol(value, NULL, 10);
					if(gd >= 0)
						cfg.glue_delay[i] = gd;
					found = true;
					break;
				}
			}
		}
		if(!found)
			ret = 0; // unknown
	} else
		ret = 0; // unknown

	if(!ret)
		printf("config.ini: unknown entry \"%s=%s\" at line %d\n", name, value, lineno);

	#undef MATCH_S
	#undef MATCH_N
	#undef MATCH_BOOL

	return ret;
}

/*
Parses config.ini.
*/
bool parse_config() {
	char filename[80];
	// first check local file
	strcpy(filename, "config.ini");
	if(access(filename, F_OK) != 0) {
		// then use standard location
		strcpy(filename, "sdmc:/3ds/fbwo/config.ini");
		if(access(filename, F_OK) != 0)
			return false;
	}

	if (ini_parse(filename, config_handler, NULL) < 0) {
		return false;
	}

	#define BOOL_STR(val) (val ? "on" : "off")

	// game
	dbgprintf("DAS delay = %ld frames\n", cfg.DAS);
	dbgprintf("invisimode: %s\n", BOOL_STR(cfg.invisimode));
	dbgprintf("hold: %s\n", BOOL_STR(cfg.hold));
	dbgprintf("ghost piece: %s\n", BOOL_STR(cfg.ghost_piece));
	dbgprintf("Hold button: %s, DAS button: %s\n",
		cfg.KEY_HOLD == KEY_L ? "L" : "R", cfg.KEY_DAS == KEY_R ? "R" : "L");
	dbgprintf("Rotation system: %s\n", cfg.ARS ? "ARS" : "SRS");
	dbgprintf("next displayed pieces no. %d\n", cfg.next_displayed);
	dbgprintf("ARE delay = %ld frames\n", cfg.ARE_delay);
	dbgprintf("DAS speed = %ld frames\n", cfg.DAS_speed);
	dbgprintf("Line clear lasts = %ld frames\n", cfg.line_clear_frames);
	dbgprintf("level = %d\n", cfg.level);
	dbgprintf("lines per level = %ld\n", cfg.lines_per_lvl);

	// theme
	dbgprintf("theme %s\n", cfg.theme_name);

	// levels
	for(int i = 0; i < 20; ++i)
		dbgprintf("lvl %d: %ld frms, %ld rows, %ld delay\n", i, cfg.frames_per_drop[i],
			cfg.rows_per_drop[i], cfg.glue_delay[i]);

	#undef BOOL_STR

	return true;
}

/*
Saves config.ini
*/
bool save_config() {
	char filename[80];
	// first check local file
	strcpy(filename, "config.ini");
	if(access(filename, F_OK) != 0) {
		// then use standard location
		strcpy(filename, "sdmc:/3ds/fbwo/config.ini");

		// create dir if necessary
		struct stat st = {0};
		if (stat("sdmc:/3ds/fbwo", &st) == -1) {
			mkdir("sdmc:/3ds/fbwo", 0777);
		}
	}

	FILE *file = fopen(filename, "w");
	if(!file)
		return false;

	#define write_int(k, v) \
		fprintf(file, "%s = %d\n", k, v)
	#define write_long(k, v) \
		fprintf(file, "%s = %ld\n", k, v)
	#define write_bool(k, v) \
		fprintf(file, "%s = %s\n", k, v ? "on" : "off")

	// game
	fprintf(file, "[game]\n");
	write_long("DAS_delay", cfg.DAS);
	write_bool("invisimode", cfg.invisimode);
	write_bool("hold", cfg.hold);
	fprintf(file, "hold_key = %s ; L or R\n",
		cfg.KEY_HOLD == KEY_L ? "L" : "R"); // FIXME::hardcoded
	write_bool("ghost_piece", cfg.ghost_piece);
	fprintf(file, "rotation_system = %s ; ars or srs\n",
		cfg.ARS ? "ars" : "srs"); // FIXME::hardcoded
	write_int("next_displayed", cfg.next_displayed);
	write_long("ARE_delay", cfg.ARE_delay);
	write_long("DAS_speed", cfg.DAS_speed);
	write_long("line_clear_frames", cfg.line_clear_frames);
	write_int("level", cfg.level);
	write_long("lines_per_level", cfg.lines_per_lvl);

	// theme
	fprintf(file, "\n[theme]\n");
	fprintf(file, "path = %s ; default or a folder name\n", cfg.theme_name);

	// levels
	for(int i = 0; i < 20; ++i) {
		fprintf(file, "\n[level %d]\n", i+1);
		write_long("frames_per_drop", cfg.frames_per_drop[i]);
		write_long("rows_per_drop", cfg.rows_per_drop[i]);
		write_long("glue_delay", cfg.glue_delay[i]);
	}

	#undef write_bool
	#undef write_long
	#undef write_int

	fclose(file);

	return true;
}

static int theme_handler(void* user, const char* section,
                         const char* name, const char* value, int lineno) {

	bool ret = 1;

	#define MATCH_S(s) strcmp(section, s) == 0
	#define MATCH_N(n) strcmp(name, n) == 0
	#define MATCH_XY(var) \
		if(MATCH_N("x")) \
			var.x = strtol(value, NULL, 10); \
		else if(MATCH_N("y")) \
			var.y = strtol(value, NULL, 10); \
		else \
			ret = 0; // unknown

	if(MATCH_S("grid")) {
		MATCH_XY(theme.grid)
	} else if(MATCH_S("next_text")) {
		MATCH_XY(theme.next_text)
	} else if(MATCH_S("score_text")) {
		MATCH_XY(theme.score_text)
	} else if(MATCH_S("hiscore_text")) {
		MATCH_XY(theme.hiscore_text)
	} else if(MATCH_S("lines_frame")) {
		MATCH_XY(theme.lines_frame)
	} else if(MATCH_S("level_frame")) {
		MATCH_XY(theme.level_frame)
	} else if(MATCH_S("hold_frame")) {
		MATCH_XY(theme.hold_frame)
	} else if(MATCH_S("offset_next")) {
		MATCH_XY(theme.offset_next)
	} else if(MATCH_S("offset_hold")) {
		MATCH_XY(theme.offset_hold)
	} else if(MATCH_S("indicators")) {
		if(MATCH_N("frames"))
			theme.indicators_frames = strtol(value, NULL, 10);
		else MATCH_XY(theme.indicators)
	} else if(MATCH_S("offsets")) {
		if(MATCH_N("lines_dig"))
			theme.digit_offset_linesy = strtol(value, NULL, 10);
		else if(MATCH_N("level_dig"))
			theme.digit_offset_levely = strtol(value, NULL, 10);
		else
			ret = 0; // unknown
	} else if(MATCH_S("next_frame")) {
		// frames
		char key[3];
		bool found = false;
		for(int i = 0; i < 6; ++i) {
			sprintf(key, "x%d", i+1);
			if(MATCH_N(key)) {
				theme.next_frame[i].x = strtol(value, NULL, 10);
				found = true;
			}
			key[0] = 'y';
			if(MATCH_N(key)) {
				theme.next_frame[i].y = strtol(value, NULL, 10);
				found = true;
			}
		}
		if(!found)
			ret = 0; // unknown
	} else
		ret = 0; // unknown

	if(!ret)
		printf("theme.ini: unknown entry \"%s=%s\" at line %d\n", name, value, lineno);

	#undef MATCH_S
	#undef MATCH_N
	#undef MATCH_XY

	return ret;
}

/*
Parses theme.ini.
*/
bool parse_theme(char* theme_name) {
	char filename[80];
	if (!get_theme_file(theme_name, "theme.ini", filename))
		return false;

	if (ini_parse(filename, theme_handler, NULL) < 0) {
		return false;
	}

	dbgprintf("grid = [%ld, %ld]\n", theme.grid.x, theme.grid.y);
	dbgprintf("next_text = [%ld, %ld]\n", theme.next_text.x, theme.next_text.y);
	for(int i = 0; i < 6; ++i)
		dbgprintf("next_frame %d = [%ld, %ld]\n", i, theme.next_frame[i].x, theme.next_frame[i].y);
	dbgprintf("score_text = [%ld, %ld]\n", theme.score_text.x, theme.score_text.y);
	dbgprintf("hiscore_text = [%ld, %ld]\n", theme.hiscore_text.x, theme.hiscore_text.y);
	dbgprintf("lines_frame = [%ld, %ld]\n", theme.lines_frame.x, theme.lines_frame.y);
	dbgprintf("level_frame = [%ld, %ld]\n", theme.level_frame.x, theme.level_frame.y);
	dbgprintf("hold_frame = [%ld, %ld]\n", theme.hold_frame.x, theme.hold_frame.y);
	dbgprintf("offset_next = [%ld, %ld]\n", theme.offset_next.x, theme.offset_next.y);
	dbgprintf("offset_hold = [%ld, %ld]\n", theme.offset_hold.x, theme.offset_hold.y);
	dbgprintf("indicators = [%ld, %ld], frame time = %ld\n", theme.indicators.x,
		theme.indicators.y, theme.indicators_frames);
	dbgprintf("lines digit offset = %ld\n", theme.digit_offset_linesy);
	dbgprintf("level digit offset = %ld\n", theme.digit_offset_levely);

	return true;
}

bool get_theme_file(const char* theme_name, const char* file, char* out) {
	const char* prefixes[] = { "romfs:", ".", "sdmc:/3ds/fbwo", NULL };

	// try theme directory
	if(strcmp("default", theme_name) != 0) {
		for(int i = 0; prefixes[i]; i++) {
			sprintf(out, "%s/%s/%s", prefixes[i], theme_name, file);
			if(access(out, F_OK) == 0)
				return true;
		}
	}

	// try default theme
	for(int i = 0; prefixes[i]; i++) {
		sprintf(out, "%s/%s", prefixes[i], file);
		if(access(out, F_OK) == 0)
			return true;
	}

	return false;
}
