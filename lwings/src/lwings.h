/* Copyright (c) 2018 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#pragma once

#pragma warning(push, 1)
#define WINVER         0x0400
#define _WIN32_WINNT   0x0400
#define _WIN32_WINDOWS 0x0400
#define _WIN32_IE      0x0400

#include <windows.h>
#include <gl/gl.h>
#pragma warning(pop)

typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

//==============================================================================

enum {
	SC_START_IN,
	SC_START,
	SC_START_OUT,
	SC_LEVEL1_IN,
	SC_LEVEL1,
	SC_LEVEL1_DANGER,
	SC_LEVEL1_LUCKY,
	SC_LEVEL1_END,
	SC_LEVEL1_OUT,
	SC_VICTORY_IN,
	SC_VICTORY,
	SC_VICTORY_OUT,
	SC_GAMEOVER_IN,
	SC_GAMEOVER,
	SC_GAMEOVER_OUT,
};

enum {
	PLAYER_SPAWN,
	PLAYER_NORMAL,
	PLAYER_DEAD,
	PLAYER_OVER
};

enum {
	ENEMY_NONE,
	ENEMY_NORMAL,
	ENEMY_STAY,
	ENEMY_KILLED,
	ENEMY_BOOM,
	ENEMY_END
};

enum {
	ENEMY_SPIRAL_RIGHT,
	ENEMY_SPIRAL_LEFT,
	ENEMY_POWERUP_LEFT,
	ENEMY_POWERUP_RIGHT,
	ENEMY_POWERUP_BONUS,
	ENEMY_FALLING_SHIFT,
	ENEMY_DANGER_DUMMY,
	ENEMY_TWISTER,
	ENEMY_SPHINX,
	ENEMY_SPHINX_BROKEN,
	ENEMY_GRAYHEAD_BIG,
	ENEMY_BLUE_THINGY,
	ENEMY_DOWN_UP,
	ENEMY_BLOCKING_BIRD,
	ENEMY_TWISTY,
	ENEMY_DIAGONAL,
	ENEMY_FLOATY,
	ENEMY_DRAGON
};

enum {
	ENEMY_TURRET,
	ENEMY_TURRET_LUCKY,
	ENEMY_GRAYHEAD_SMALL,
	ENEMY_GARGOYLE
};

enum {
	INPUT_A = 1 << 0,
	INPUT_B = 1 << 1,
	INPUT_START  = 1 << 2,
	INPUT_SELECT = 1 << 3
};

struct frame {
	float ox, oy, u, v, w, h, tw, th;
};

struct sprite {
	float x, y, w, h;
	float time_adv;
	float time_cur;
	float frames;
};

struct bullet {
	struct sprite;
	float vx, vy, ty;
	int state;
	int id;
};

struct player {
	struct sprite;
	struct frame *fr;
	int state;
	int power;
	int lives;
	int score;
	float flicker;
	double bullet_next;
	double bomb_next;
};

struct enemy {
	struct sprite;
	struct frame *fr;
	float vx, vy, vel;
	int state;
	float hp;
	int id;
	int step;
	int score;
	double timer;
	double bullet_next;
};

void sprite_frame_adv(double dt, void *spr);

void draw_quad_wire(float x, float y, float w, float h);
void draw_quad(float x, float y, float w, float h);
void draw_frame(float x, float y, struct frame *f);
void draw_framex(float x, float y, struct frame *f, int flipx, int flipy);
void draw_text_ui(float x, float y, int id, char *fmt, ...);
void draw_text(float x, float y, char *fmt, ...);
void draw_color_fill(float *col3, float alpha);
void draw_fade(float alpha);
void draw_fancy_border(void);
void draw_fade_in(int next);
void draw_fade_out(int next);
void draw_player(double dt, int id, struct player *p);
void boom_animation(double dt, struct enemy *e);

void lwings_game(double dt);
