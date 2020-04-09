/* Copyright (c) 2018 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "lwings.h"
#include "bass.h"

#define PI 3.14159265358979323846

extern GLuint tex_atlas, tex_bullets, tex_enemies, tex_player;
extern HSTREAM gameover_music;
extern HSTREAM hiscore_music;
extern HSTREAM level1_music;
extern HSTREAM warning_music;
extern HSTREAM boss_music;
extern HSTREAM victory_music;
extern HSAMPLE sndfx[18];
extern HCHANNEL sndch[18];
extern BYTE keyboard[256];

int highscore;
int two_players;
int game_scene = SC_START_IN;
float map_offset = 5040.0f;
float fall_x, fall_y, fall_done;
double time_accum;
double bg_flashing;

//00 - powerup
//01 - twisty ship
//02 - falling ship, turns half screen towards player
//03 - big thing (sphinx?)
//04 - gray heads, starts ground
//05 - blue thingy, lots of hip
//06 - down and up ships
//07 - twisty ship 2, slowish
//08 - diagonal thingies
//09 - floaty thing
//10 - dragon

struct frame enemyf0[4] = {
	{0, 0, 168, 0, 16, 16, 256, 128},
	{0, 0, 168,16, 16, 16, 256, 128},
	{0, 0, 168,32, 16, 16, 256, 128},
	{4, 4, 184,24,  8,  8, 256, 128}};

struct frame enemyf1[4] = {
	{4, 0,  8, 0,  8, 16, 256, 128},
	{0, 0, 16, 0, 16, 16, 256, 128},
	{0, 0, 32, 0, 16, 16, 256, 128},
	{0, 0, 16, 0, 16, 16, 256, 128}};

struct frame enemyf2[3] = {
	{0, 0, 48, 0, 16, 16, 256, 128},
	{1, 0, 64, 0, 16, 16, 256, 128},
	{2, 0, 80, 0, 16, 16, 256, 128}};

struct frame enemyf3[2] = {
	{-8, -8, 24, 32, 24, 24, 256, 128},
	{-8, -8, 48, 32, 24, 24, 256, 128}};

struct frame enemyf4[2] = {
	{0, 0, 0, 16, 16, 16, 256, 128},
	{0, 0, 0, 32, 24, 24, 256, 128}};

struct frame enemyf5[4] = {
	{0, 0, 96, 0, 16, 16, 256, 128},
	{0, 0,112, 0, 16, 16, 256, 128},
	{0, 0,128, 0, 16, 16, 256, 128},
	{0, 0,144, 0, 16, 16, 256, 128}};

struct frame enemyf6[4] = {
	{0, 0, 16, 16, 16, 16, 256, 128},
	{0, 0, 32, 16, 16, 16, 256, 128},
	{0, 0, 48, 16, 16, 16, 256, 128}};

struct frame enemyf7[4] = {
	{0, 0, 64, 16, 16, 16, 256, 128},
	{2, 0, 80, 16, 11, 16, 256, 128},
	{4, 0, 91, 16,  8, 16, 256, 128},
	{3, 0, 99, 16, 11, 16, 256, 128}};

struct frame enemyf8[4] = {
	{0, 0, 110, 16, 16, 16, 256, 128},
	{0, 0, 126, 16, 16, 16, 256, 128},
	{0, 0, 142, 16, 16, 16, 256, 128},
	{0, 0, 142, 32, 16, 16, 256, 128}};

struct frame enemyf9[3] = {
	{0, 0,  72, 32, 16, 16, 256, 128},
	{0, 0,  88, 32, 16, 16, 256, 128},
	{0, 0, 104, 32, 16, 16, 256, 128}};

struct frame enemyf10[4] = {
	{ 0-12, 0,   0, 56, 48, 72, 256, 128},
	{ 6-12, 0,  48, 56, 36, 72, 256, 128},
	{12-12, 0,  84, 56, 24, 72, 256, 128},
	{ 6-12, 0, 108, 56, 36, 72, 256, 128}};

struct frame enemyf11[4] = {
	{ 8, 8, 248, 32,  8,  8, 256, 128},
	{ 4, 4, 240, 16, 16, 16, 256, 128},
	{ 0, 0, 216, 16, 24, 24, 256, 128},
	{ 0, 0, 192, 16, 24, 24, 256, 128}};

struct frame playerf[24] = {
	{0, 0,  0,  0, 32, 32, 256, 64}, // P1
	{0, 0, 32,  0, 32, 32, 256, 64},
	{0, 0, 64,  0, 32, 32, 256, 64},
	{0, 0, 96,  0, 32, 32, 256, 64},

	{0, 0,  0, 32, 32, 32, 256, 64}, // P2
	{0, 0, 32, 32, 32, 32, 256, 64},
	{0, 0, 64, 32, 32, 32, 256, 64},
	{0, 0, 96,  0, 32, 32, 256, 64},

	{0, 0,160,  0, 32, 32, 256, 64}, // Full power
	{0, 0,192,  0, 32, 32, 256, 64},
	{0, 0,224,  0, 32, 32, 256, 64},
	{0, 0, 96,  0, 32, 32, 256, 64},

	{0, 0,160, 32, 32, 32, 256, 64},
	{0, 0,192, 32, 32, 32, 256, 64},
	{0, 0,224, 32, 32, 32, 256, 64},
	{0, 0, 96,  0, 32, 32, 256, 64},

	{ 4, 5,136,  0, 24, 20, 256, 64}, // P1 Fall
	{ 8, 5, 96, 32, 16, 16, 256, 64},
	{10, 5,112, 32, 12, 16, 256, 64},
	{10, 5,124, 32, 12, 16, 256, 64},

	{ 4, 5,136, 32, 24, 20, 256, 64}, // P2 Fall
	{ 8, 5, 96, 48, 16, 16, 256, 64},
	{10, 5,112, 48, 12, 16, 256, 64},
	{10, 5,124, 48, 12, 16, 256, 64}};

struct enemy enemyt[12] = {//                vx vy  v  st hp id sp sc   t  bn
	{0, 0, 16, 16, 60.0f/9.0f, 0, 3, enemyf0, 0, 0, 0, 1, 0, 0, 0, 300, 0, 0}, //00 - powerup
	{0, 0, 16, 16, 60.0f/9.0f, 0, 4, enemyf1, 0, 0, 0, 1, 0, 0, 0, 300, 0, 0}, //01 - twisty ship
	{0, 0, 16, 16, 60.0f/9.0f, 0, 1, enemyf2, 0, 0, 0, 1, 0, 0, 0, 300, 0, 0}, //02 - falling ship, turns half screen towards player
	{0, 0, 24, 24, 60.0f/9.0f, 0, 1, enemyf3, 0, 0, 0, 1, 0, 0, 0, 800, 0, 0}, //03 - big thing (sphinx?)
	{0, 0, 16, 16, 60.0f/9.0f, 0, 1, enemyf4, 0, 0, 0, 1, 0, 0, 0, 800, 0, 0}, //04 - gray heads, starts ground
	{0, 0, 16, 16, 60.0f/9.0f, 0, 4, enemyf5, 0, 0, 0, 1, 0, 0, 0, 400, 0, 0}, //05 - blue thingy, lots of hip
	{0, 0, 16, 16, 60.0f/9.0f, 0, 1, enemyf6, 0, 0, 0, 1, 0, 0, 0, 300, 0, 0}, //06 - down and up ships
	{0, 0, 16, 16, 60.0f/9.0f, 0, 4, enemyf7, 0, 0, 0, 1, 0, 0, 0, 400, 0, 0}, //07 - twisty ship 2, slowish
	{0, 0, 16, 16, 60.0f/9.0f, 0, 4, enemyf8, 0, 0, 0, 1, 0, 0, 0, 400, 0, 0}, //08 - diagonal thingies
	{0, 0, 16, 16, 60.0f/9.0f, 0, 3, enemyf9, 0, 0, 0, 1, 0, 0, 0, 300, 0, 0}, //09 - floaty thing
	{0, 0, 24, 72, 60.0f/11.0f,0, 4,enemyf10, 0, 0, 0, 1, 0, 0, 0,3000, 0, 0}, //10 - dragon
	{0, 0, 24, 24, 60.0f/11.0f,0, 1,enemyf11, 0, 0, 0, 1, 0, 0, 0,  0, 0, 0}};//11 - twister

struct frame bulletf[5] = {
	{0, 0,  8,  0,  8,  8, 256, 16}, // player_power_1
	{0, 0, 16,  0, 24, 40, 256, 16}, // player_power_2
	{0, 0, 40,  0, 16, 32, 256, 16}, // player_power_3
	{0, 0, 56,  0, 16, 16, 256, 16}, // player_power_4
	{0, 0, 88,  0, 32,  8, 256, 16}};// player_power_5

struct frame bulletenemy = {0, 0, 0,  8,  8,  8, 256, 16}; // enemy_bullet
struct frame bullethit   = {0, 0, 8,  8,  8,  8, 256, 16}; // player_bomb_hit

#define PLAYER_BULLETS 8
#define ENEMY_BULLETS 8

struct bullet pbomb[2];
struct bullet pbullet[2][PLAYER_BULLETS];
struct bullet ebullet[ENEMY_BULLETS];

struct player player0 = {128, 240, 32, 32, 60.0f/9.0f, 0, 3};
struct player player1, player2;

#define GROUND_ENEMIES 8
#define AIR_ENEMIES 24

struct enemy genemy[GROUND_ENEMIES];
struct enemy aenemy[AIR_ENEMIES];
struct enemy dragon[3];

struct pinput {
	float x, y;
	unsigned buttons;
} input[2];

void play_sfx(int id)
{
	BASS_ChannelPlay(sndch[id], TRUE);
	BASS_ChannelSetAttribute(sndch[id], BASS_ATTRIB_PAN, 0);
}

void play_sfx_pan(int id, float x)
{
	BASS_ChannelPlay(sndch[id], TRUE);
	BASS_ChannelSetAttribute(sndch[id], BASS_ATTRIB_PAN, (x-128.0f)/512.0f);
}

void play_music(HSTREAM music)
{
	if(BASS_ChannelIsActive(music) != BASS_ACTIVE_PLAYING) {
		BASS_ChannelSetAttribute(music, BASS_ATTRIB_VOL, 1);
		BASS_ChannelPlay(music, TRUE);
	}
}

void fadeout_music(HSTREAM music, DWORD time)
{
	BASS_ChannelSlideAttribute(music, BASS_ATTRIB_VOL|BASS_SLIDE_LOG, -1, time);
}

void clear_level_data(void)
{
	int i;

	player1.state = 0;
	player2.state = 0;

	pbomb[0].state = 0;
	pbomb[1].state = 0;

	for(i = 0; i < PLAYER_BULLETS; i++) pbullet[0][i].state = 0;
	for(i = 0; i < PLAYER_BULLETS; i++) pbullet[1][i].state = 0;
	for(i = 0; i < ENEMY_BULLETS;  i++) ebullet[i].state = 0;
	for(i = 0; i < GROUND_ENEMIES; i++) genemy[i].state = 0;
	for(i = 0; i < AIR_ENEMIES;    i++) aenemy[i].state = 0;
}

void bind_texture(GLuint tex)
{
	static GLuint prev;
	if(tex != prev) {
		glBindTexture(GL_TEXTURE_2D, tex);
		prev = tex;
	}
}

void check_input(void)
{
	ZeroMemory(input, sizeof(input));

	if(keyboard[VK_UP])    input[0].y += -1.0f;
	if(keyboard[VK_DOWN])  input[0].y +=  1.0f;
	if(keyboard[VK_LEFT])  input[0].x += -1.0f;
	if(keyboard[VK_RIGHT]) input[0].x +=  1.0f;

	if(keyboard[VK_RETURN]) input[0].buttons |= INPUT_START;
	if(keyboard[VK_BACK])   input[0].buttons |= INPUT_SELECT;

	if(keyboard['Z']) input[0].buttons |= INPUT_B;
	if(keyboard['X']) input[0].buttons |= INPUT_A;

	if(keyboard['W']) input[1].y += -1.0f;
	if(keyboard['S']) input[1].y +=  1.0f;
	if(keyboard['A']) input[1].x += -1.0f;
	if(keyboard['D']) input[1].x +=  1.0f;

	if(keyboard[VK_RETURN]) input[1].buttons |= INPUT_START;
	if(keyboard[VK_BACK])   input[1].buttons |= INPUT_SELECT;

	if(keyboard['F']) input[1].buttons |= INPUT_B;
	if(keyboard['G']) input[1].buttons |= INPUT_A;
}


#include "map_level1.c"

int ground_enemy_count(void)
{
	int i, count = 0;
	for(i = 0; i < GROUND_ENEMIES; i++)
		if(genemy[i].state == ENEMY_NORMAL) count++;
	return count;
}

void add_ground_enemy(int x, int y, int id)
{
	int ge_free = -1;
	int i;

	for(i = 0; i < GROUND_ENEMIES; i++) {
		if(!genemy[i].state) {
			ge_free = i;
			break;
		}
	}

	if(ge_free >= 0) {
		struct enemy *e = &genemy[ge_free];
		e->bullet_next = time_accum + 1.0;
		e->x     = x * 16.0f;
		e->y     = y * 16.0f;
		e->vel   = y * 16.0f;
		e->w     = 16.0f;
		e->h     = 16.0f;
		e->state = 1;
		e->hp    = 1;
		e->step  = 0;
		e->score = 300;

		switch(id) {
		case 33:
			e->id = ENEMY_TURRET;
			e->vx = 4.0f;
			e->vy = 4.0f;
			break;
		case 34:
			e->id = ENEMY_TURRET_LUCKY;
			e->vx = 4.0f;
			e->vy = 4.0f;
			break;
		case 86:
			e->id = ENEMY_GRAYHEAD_SMALL;
			e->x += 8.0f;
			e->score = 800;
			break;
		case 98:
			e->id = ENEMY_GARGOYLE;
			e->hp = 3;
			e->vel -= 8.0f;
			if(x < 8) {
				e->vx = 6.0f;
				e->vy = 9.0f;
			} else {
				e->vx = 2.0f;
				e->vy = 9.0f;
			}
			break;
		}
	}
}

void enemy_blocking_bird(float x, float y);

void draw_map(void)
{
	struct frame tile = { 0, 0, 0, 0, 16, 16, 256, 256};
	int x, y;
	static int last;
	int min = (int)map_offset / 16;
	int max = min + 16;
	if(max > 330) max = 330;

	if(last != min) {
		y = min;
		for(x = 0; x < 16; x++) {
			int id = map[y][x];
			if(id == 33 || id == 34 || id == 86 || id == 98)
				add_ground_enemy(x, y, id);
			else if(id == 100)
				enemy_blocking_bird(x * 16.0f, y * 16.0f - 32.0f);
		}
		last = min;
	};

	for(y = min; y < max; y++) {
		for(x = 0; x < 16; x++) {
			int id = map[y][x];
			tile.u = (float)((id & 0xF) << 4);
			tile.v = (float)(id & ~0xF);
			draw_frame(x * 16.0f, y * 16.0f, &tile);
		}
	}
}

int start_mode;

void draw_start(void)
{
	struct frame legendary = { 0, 0, 72, 184, 104, 16, 256, 256};
	struct frame wings = { 0, 0, 72, 200, 184, 32, 256, 256};

	glColor3f(0, 0, 0);
	glDisable(GL_TEXTURE_2D);
	draw_quad(0, 0, 256, 240);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);

	draw_frame(80.0f, 48.0f, &legendary);
	draw_frame(40.0f, 72.0f, &wings);

	draw_text(0, 0, "0");
	draw_text(80, 120.0f + start_mode * 16.0f, "*");
	draw_text(96, 120, "1 Player");
	draw_text(96, 136, "2 Players");
	draw_text(8,  176, "TM and @1988 CAPCOM U.S.A.,Inc.");
	draw_text(88, 192, "Licensed by");
	draw_text(32, 208, "Nintendo of America Inc.");

	glColor3f(0.2f, 0.2f, 0.2f);
	draw_text(56, 8, "* Demo by KrossX *");
	glColor3f(1, 1, 1);
}

int check_toggle(int val, int *staticvar)
{
	if( val && !*staticvar) { *staticvar = 1; return 1; } else
	if(!val &&  *staticvar) { *staticvar = 0; }
	return 0;
}

void scene_start_in(double dt)
{
	bind_texture(tex_atlas);
	draw_start();
	draw_fade_in(SC_START);
}

void scene_start(double dt)
{
	static int menu_up;
	static int menu_down;

	check_input();
	if(check_toggle(input[0].y < 0, &menu_up))  { start_mode--; play_sfx(1); }
	if(check_toggle(input[0].y > 0, &menu_down)){ start_mode++; play_sfx(1); }

	start_mode &= 1;

	bind_texture(tex_atlas);
	draw_start();

	if(input[0].buttons & INPUT_START) {
		play_sfx(12);

		clear_level_data();

		game_scene = SC_START_OUT;
		time_accum = 0;

		two_players = start_mode;

		player1 = player0;
		player2 = player0;

		player1.fr = &playerf[0];
		player1.lives   = 2;
		player1.state   = PLAYER_SPAWN;
		player1.flicker = 3.0f;

		player2.fr = &playerf[4];
		player2.lives   = 2;
		player2.state   = PLAYER_SPAWN;
		player2.flicker = 3.0f;

		if(two_players) {
			player1.x -= 16.0f;
			player2.x += 16.0f;
		}

		//map_offset = 2 * 16.0f; //5040.0f; //========================================[MAPOFF]
		map_offset = 5040.0f;
	}
}

void scene_start_out(double dt)
{
	bind_texture(tex_atlas);
	draw_start();
	draw_fade_out(SC_LEVEL1_IN);
}

void scene_level1_in(double dt)
{
	bind_texture(tex_atlas);
	glColor3f(1, 1, 1);

	glPushMatrix();
		glTranslatef(0, -map_offset, 0);
		draw_map();
	glPopMatrix();

	play_music(level1_music);
	draw_fade_in(SC_LEVEL1);
}

float clampf(float val, float min, float max)
{
	return val < min? min : val > max ? max : val;
}

void towards_player(float *dx, float *dy, struct enemy *e, int is_bullet)
{
	float x, y, len = 999000;
	float x2, y2, len2 = 999000;
	float ox = 4, oy = 4;

	if(!is_bullet) {
		ox = e->w * 0.5f;
		oy = e->h * 0.5f;
	}

	if(player1.state == PLAYER_NORMAL) {
		x = player1.x - e->x - ox;
		y = player1.y - e->y + 8.0f - oy;
		len = x * x + y * y;
	}

	if(two_players &&  player2.state == PLAYER_NORMAL) {
		x2 = player2.x - e->x - ox;
		y2 = player2.y - e->y + 8.0f - oy;
		len2 = x2 * x2 + y2 * y2;
		if(len2 < len) { x = x2; y = y2; len = len2; }
	}

	if(len < 999000) { len = (float)sqrt(len); *dx = x / len; *dy = y / len; }
	else { *dx = 0; *dy = 1; }
}

void shoot_dragon(struct enemy *e)
{
	int eb_free[ENEMY_BULLETS];
	int eb_slots = 0;
	int i;

	if(e->bullet_next > 0)
		return;

	for(i = 0; i < ENEMY_BULLETS; i++) {
		if(!ebullet[i].state) {
			eb_free[eb_slots] = i;
			eb_slots++;
		}
	}

	if(eb_slots >= 3) {
		float vx[3] = {-42,  0, 42}; // 22° spread
		float vy[3] = {113,122,113};

		if(e->vy > 0) {
			e->bullet_next = 5.1;
			e->vy = 0;
		} else {
			e->bullet_next = 0.5;
			e->vy = 1;
		}

		for(i = 0; i < 3; i++) {
			struct bullet *b = &ebullet[eb_free[--eb_slots]];
			b->x     = e->x + 12 - 4;
			b->y     = e->y + 72;
			b->w     = 8;
			b->h     = 16;
			b->vx    = vx[i];
			b->vy    = vy[i];
			b->ty    = 0;
			b->state = 1;
			b->id    = ENEMY_DRAGON;
		}

		play_sfx(17);
	}
}

void shoot_ebullet(struct enemy *e)
{
	int eb_free = -1;
	int i;

	if(time_accum < e->bullet_next)
		return;

	for(i = 0; i < ENEMY_BULLETS; i++) {
		if(!ebullet[i].state) {
			eb_free = i;
			break;
		}
	}

	if(eb_free >= 0) {
		struct bullet *b = &ebullet[eb_free];
		towards_player(&b->vx, &b->vy, e, 1);

		if(e->id == 98) {
			e->bullet_next = time_accum + (e->step ? 0.5 : 3.0);
			e->step++; e->step &= 3;
		} else {
			e->bullet_next = time_accum + 124.0 / 60.0;
		}

		b->x     = e->x;
		b->y     = e->y;
		b->w     = 6;
		b->h     = 6;
		b->vx   *= 124.0f;
		b->vy   *= 124.0f;
		b->state = 1;
		b->id    = e->id;
	}
}

void shoot_pbomb(struct player *p, struct bullet *pb)
{
	float bomb_spd[5] = {117, 115, 230, 240, 360};
	float bomb_len[5] = {76, 110, 110, 220, 220};

	if(time_accum < p->bomb_next || pb->state)
		return;

	p->bomb_next = time_accum + 0.2;
	pb->x  = p->x;
	pb->y  = p->y;
	pb->vy = bomb_spd[p->power];
	pb->ty = bomb_len[p->power];
	pb->state = 1;
	//pb->id = 0;
	play_sfx_pan(5, p->x);
}

void shoot_pgun(struct player *p, struct bullet *pb)
{
	static unsigned counter;

	int pb_free[PLAYER_BULLETS];
	int pb_used = 0;
	int i, idx = 0;
	int gunid = p->power;

	if(time_accum < p->bullet_next)
		return;

	for(i = 0; i < PLAYER_BULLETS; i++) {
		if(pb[i].state) pb_used++;
		else pb_free[idx++] = i;
	}

	switch(gunid) {
	case 0:
		if(pb_used < 3) {
			struct bullet *b = &pb[pb_free[0]];
			p->bullet_next   = time_accum + 0.1;
			b->x     = p->x - 4.0f;
			b->y     = p->y;
			b->w     = 8.0f;
			b->h     = 8.0f;
			b->vy    = 464.0f;
			b->state = 1;
			b->id    = gunid;
			play_sfx_pan(1, p->x);
		} break;
	case 1:
		if(pb_used < 2) {
			struct bullet *b = &pb[pb_free[0]];
			p->bullet_next = time_accum + 0.08;
			b->x     = p->x - 12.0f;
			b->y     = p->y - 32.0f;
			b->w     = 24.0f;
			b->h     = 40.0f;
			b->vy    = 440.0f;
			b->ty    = 0.0f;
			b->state = 1;
			b->id    = gunid;
			play_sfx_pan(2, p->x);
		} break;
	case 2:
		if(pb_used < 1) {
			struct bullet *b = &pb[pb_free[0]];
			p->bullet_next   = time_accum + 0.1;
			b->x     = p->x - 8.0f;
			b->y     = p->y - 24.0f;
			b->w     = 16.0f;
			b->h     = 32.0f;
			b->vy    = 1370.0f;
			b->state = 3;
			b->id    = gunid;
			play_sfx_pan(15, p->x);
		} break;
	case 3:
		if(pb_used < 1) {
			struct bullet *b = &pb[pb_free[0]];
			struct bullet *b1 = &pb[pb_free[1]];
			struct bullet *b2 = &pb[pb_free[2]];
			p->bullet_next   = time_accum + 0.1;
			b->x     = p->x - 8.0f;
			b->y     = p->y;
			b->w     = 16.0f;
			b->h     = 16.0f;
			b->vx    = 0;
			b->vy    = 400.0f;
			b->ty    = 0;
			b->state = 1;
			b->id    = gunid;

			*b1 = *b2 = *b;
			b1->vx = -p->x;
			b2->vx =  p->x;

			play_sfx_pan(14, p->x);
		} break;
	case 4:
		if(pb_used < 3) {
			struct bullet *b = &pb[pb_free[0]];
			p->bullet_next   = time_accum + 0.07;
			b->x     = p->x - 16.0f;
			b->y     = p->y;
			b->w     = 32.0f;
			b->h     = 8.0f;
			b->vy    = 400.0f;
			b->state = 1;
			b->id    = gunid;
			play_sfx_pan(16, p->x);
		} break;
	}
}

void player_bombs_step(double dt, struct bullet *pb)
{
	struct frame bomb = {0, 0, 0, 0, 8, 8, 256, 16};
	struct frame boom = {0, 0, 8, 8, 8, 8, 256, 16};
	float step;

	if(!pb->state)
		return;

	if(pb->state == 1) {
		draw_frame(pb->x - 4.0f, pb->y, &bomb);
		step = (float)(dt * pb->vy);
		pb->y  -= step;
		pb->ty -= step;
		if(pb->y < 0) pb->state = 0;
		else if(pb->ty < 0) {
			pb->state = 2;
			pb->ty = 4.0f / 60.0f;
		}
	} else {
		draw_frame(pb->x - 4.0f, pb->y, &boom);
		pb->ty -= (float)dt;
		if(pb->ty < 0) pb->state = 0;
	}
}

void player_bullets_step(double dt, struct player *p, struct bullet *pb)
{
	int i;
	for(i = 0; i < PLAYER_BULLETS; i++) {
		if(!pb[i].state) continue;

		switch(pb[i].id) {
		case 0: {
			draw_frame(pb[i].x, pb[i].y, &bulletf[0]);
			pb[i].y -= (float)(dt * pb[i].vy);
			if(pb[i].y < 0)pb[i].state = 0;
			} break;
		case 1: {
			struct frame bf = bulletf[1];
			bf.v = pb[i].ty;
			draw_frame(pb[i].x, pb[i].y, &bf);
			pb[i].ty -= (float)(dt * pb[i].vy);
			pb[i].y  -= (float)(dt * pb[i].vy);
			if(pb[i].y < 0) pb[i].state = 0;
			} break;
		case 2: {
			draw_frame(pb[i].x, pb[i].y, &bulletf[2]);
			pb[i].y -= (float)(dt * pb[i].vy);
			if(pb[i].y < 0) {
				pb[i].state--;
				if(pb[i].state > 0) {
					pb[i].x = p->x - 8.0f;
					pb[i].y = p->y - 24.0f;
				} else
					pb[i].state = 0;
			}} break;
		case 3: {
			struct frame bf = bulletf[3];
			bf.u += ((int)(pb[i].ty * 9.0f)&1) * 16.0f;
			draw_frame(pb[i].x, pb[i].y, &bf);
			pb[i].ty += (float)dt;
			pb[i].y -= (float)(dt * pb[i].vy);
			if(pb[i].y < 0) pb[i].state = 0;
			else if(pb[i].vx > 0) pb[i].x = pb[i].vx - (float)sin(pb[i].ty * 20.0) * 64.0f;
			else if(pb[i].vx < 0) pb[i].x = (float)sin(pb[i].ty * 20.0) * 64.0f - pb[i].vx;
			} break;
		case 4: {
			draw_frame(pb[i].x, pb[i].y, &bulletf[4]);
			pb[i].y -= (float)(dt * pb[i].vy);
			if(pb[i].y < 0) pb[i].state = 0;
			} break;
		}
	}
}

void enemy_bullets_step(double dt)
{
	struct frame bullet  = {-1, -1, 0, 8, 8, 8, 256, 16};
	struct frame fire = {0, 0, 208, 0, 8, 16, 256, 16};
	int i;

	for(i = 0; i < ENEMY_BULLETS; i++) {
		if(!ebullet[i].state) continue;

		if(ebullet[i].id == ENEMY_DRAGON) {
			fire.u = 208.0f + ((int)ebullet[i].ty%3)*16.0f;
			draw_frame(ebullet[i].x, ebullet[i].y, &fire); fire.u += 8.0f;
			draw_frame(ebullet[i].x, ebullet[i].y - 16, &fire);
			ebullet[i].ty += (float)(dt*8);
		} else {
			draw_frame(ebullet[i].x, ebullet[i].y, &bullet);
		}

		ebullet[i].x += (float)(ebullet[i].vx * dt);
		ebullet[i].y += (float)(ebullet[i].vy * dt);

		if(ebullet[i].x < 0 || ebullet[i].x > 256.0f) ebullet[i].state = 0;
		if(ebullet[i].y < 0 || ebullet[i].y > 240.0f) ebullet[i].state = 0;
	}
}

void hardcoded_ground_step(double dt, struct enemy *e)
{
	struct frame turret   = {0, 0, 224, 96, 16, 16, 256, 128};
	struct frame gargoyle = {0,-3, 240, 75, 16, 21, 256, 128};

	e->y = e->vel - map_offset;
	if(e->y > 240.0f) e->state = 0;

	switch(e->id){
	case ENEMY_TURRET:
	case ENEMY_TURRET_LUCKY:
		draw_frame(e->x, e->y, &turret);
		e->x += e->vx; e->y += e->vy;
		shoot_ebullet(e);
		e->x -= e->vx; e->y -= e->vy;
		break;

	case ENEMY_GRAYHEAD_SMALL:
		draw_frame(e->x, e->y, &enemyf4[0]);
		if(e->y > 58.0f) {
			int i;
			for(i = 0; i < AIR_ENEMIES; i++)
				if(!aenemy[i].state) {
					struct enemy *ne = &aenemy[i];
					*ne = *e;
					ne->id  = ENEMY_GRAYHEAD_BIG;
					ne->fr  = enemyf4;
					ne->vel = 122.0f;
					ne->hp  = 100.0f;
					ne->step = 0;
					ne->timer = 0;
					towards_player(&ne->vx, &ne->vy, ne, 0);
					break;
				}
			e->state = 0;
		}
		break;

	case ENEMY_GARGOYLE:
		if(e->x < 128.0f) draw_frame(e->x, e->vel - map_offset, &gargoyle);
		else draw_framex(e->x, e->vel - map_offset, &gargoyle, 1, 0);
		e->x += e->vx; e->y += e->vy;
		shoot_ebullet(e);
		e->x -= e->vx; e->y -= e->vy;
		break;
	}
}

void enemy_twister(void);

void ground_enemy_step(double dt)
{
	int i;

	for(i = 0; i < GROUND_ENEMIES; i++) {
		switch(genemy[i].state) {
		case ENEMY_NONE:
			continue;
		case ENEMY_NORMAL:
			hardcoded_ground_step(dt, &genemy[i]);
			continue;
		case ENEMY_STAY:
			continue;
		case ENEMY_KILLED:
			if(genemy[i].id == ENEMY_TURRET_LUCKY) {
				fall_x = genemy[i].x - 4.0f;
				fall_y = genemy[i].y + map_offset + 4.0f;
				fall_done = 0;
				enemy_twister();
			}
			genemy[i].time_cur = 0;
			genemy[i].state = ENEMY_BOOM;
			continue;
		case ENEMY_BOOM:
			genemy[i].y = genemy[i].vel - map_offset;
			boom_animation(dt, &genemy[i]);
			continue;
		case ENEMY_END:
			genemy[i].state = 0;
			continue;
		}
	}
}

void hardcoded_stay(double dt, struct enemy *e) {
	struct frame headf[3] = {
		{0, 0, 160, 112, 96, 16, 256, 128},
		{0, 0, 240,  96, 16, 16, 256, 128},
		{0, 0, 240, 102, 10, 10, 256, 128}};

	switch(e->id) {
	case ENEMY_BLOCKING_BIRD:
		e->y = e->vy - map_offset;
		if(e->y > 240.0f) e->state = 0;
		return;

	case ENEMY_POWERUP_BONUS:
		e->y += (float)(30.0 * dt);
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		draw_frame(e->x, e->y, &e->fr[3]);
		sprite_frame_adv(dt, e);
		if(e->y > 240.0f) e->state = 0;
		return;

	case ENEMY_DANGER_DUMMY:
		e->y = 233.0f * 16.0f - map_offset;
		switch(e->step) {
		case 0:
			if(e->y > 49.0f) {
				map[233][ 5] = 35;
				map[233][ 6] = 35;
				map[233][ 7] = 35;
				map[233][ 8] = 35;
				map[233][ 9] = 35;
				map[233][10] = 35;
				e->step = 1;
			}
			break;
		case 1:
			draw_framex(e->x - 48.0f, e->y, &headf[1], 1, 0);
			draw_frame(e->x + 32.0f, e->y, &headf[1]);
			draw_frame(e->x - 48.0f, e->y + 8.0f, &headf[0]);
			if(e->y > 55.0f) e->step = 2;
			break;

		case 2:
			draw_framex(e->x - 48.0f, e->y, &headf[1], 1, 0);
			draw_frame(e->x + 32.0f, e->y, &headf[1]);
			draw_framex(e->x - 42.0f, e->y + 16.0f, &headf[2], 1, 0);
			draw_frame(e->x + 32.0f, e->y + 16.0f, &headf[2]);
			draw_frame(e->x - 48.0f, e->y + 16.0f, &headf[0]);
			if(e->y > 60.0f) e->step = 3;
			break;

		case 3:
			draw_framex(e->x - 48.0f, e->y, &headf[1], 1, 0);
			draw_frame(e->x + 32.0f, e->y, &headf[1]);
			draw_framex(e->x - 42.0f, e->y + 16.0f, &headf[2], 1, 0);
			draw_frame(e->x + 32.0f, e->y + 16.0f, &headf[2]);
			draw_framex(e->x - 42.0f, e->y + 26.0f, &headf[2], 1, 0);
			draw_frame(e->x + 32.0f, e->y + 26.0f, &headf[2]);
			draw_frame(e->x - 48.0f, e->y + 24.0f, &headf[0]);
			if(e->y > 70.0f) e->step = 4;
			break;

		case 4:
			draw_framex(e->x - 48.0f, e->y, &headf[1], 1, 0);
			draw_frame(e->x + 32.0f, e->y, &headf[1]);
			draw_framex(e->x - 42.0f, e->y + 16.0f, &headf[2], 1, 0);
			draw_frame(e->x + 32.0f, e->y + 16.0f, &headf[2]);
			draw_framex(e->x - 42.0f, e->y + 26.0f, &headf[2], 1, 0);
			draw_frame(e->x + 32.0f, e->y + 26.0f, &headf[2]);
			draw_frame(e->x - 48.0f, e->y + 32.0f, &headf[0]);
			if(e->y > 240.0f) {
				map[233][ 5] = 121;
				map[233][ 6] = 122;
				map[233][ 7] = 124;
				map[233][ 8] = 124;
				map[233][ 9] = 125;
				map[233][10] = 126;
				e->state = 0;
			}
			break;
		}
		return;
	}
}

void hardcoded_step(double dt, struct enemy *e)
{
	switch(e->id) {
	case ENEMY_SPIRAL_RIGHT: {
		static int set;
		static float vx, vy;
		e->x += (float)(e->vx * e->vel * dt);
		e->y += (float)(e->vy * e->vel * dt);
		switch(e->step) {
		case 0:
			if(e->y >= 80.0f) {
				set = 1;
				e->step  = 1;
				e->timer = 1.570796;
			}
			break;
		case 1:
			e->vx = (float)cos(e->timer);
			e->vy = (float)sin(e->timer);
			e->timer += dt * 6.6371;
			if(e->timer >= 9.424778) {
				if(set) { towards_player(&vx, &vy, e, 0); set = 0; }
				e->vx = vx;
				e->vy = vy;
				e->step = 2;
			}
			break;
		case 2:
			if(e->x < -e->w || e->x > 256.0f ||
				e->y < -e->h || e->y > 240.0f)
				e->state = 0;
		}
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e); }
		return;
	case ENEMY_SPIRAL_LEFT: {
		static int set;
		static float vx, vy;
		e->x += (float)(e->vx * e->vel * dt);
		e->y += (float)(e->vy * e->vel * dt);
		switch(e->step) {
		case 0:
			if(e->y >= 80.0f) {
				set = 1;
				e->step  = 1;
				e->timer = 1.570796;
			}
			break;
		case 1:
			e->vx = -(float)cos(e->timer);
			e->vy = (float)sin(e->timer);
			e->timer += dt * 6.6371;
			if(e->timer >= 9.424778) {
				if(set) { towards_player(&vx, &vy, e, 0); set = 0; }
				e->vx = vx;
				e->vy = vy;
				e->step = 2;
			}
			break;
		case 2:
			if(e->x < -e->w || e->x > 256.0f ||
				e->y < -e->h || e->y > 240.0f)
				e->state = 0;
		}
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e); }
		return;

	case ENEMY_POWERUP_LEFT:
		switch(e->step) {
		case 0:
			e->x = (float)cos(e->timer) * 135.0f + 128.0f;
			e->y = (float)sin(e->timer) * 135.0f + 172.0f;
			e->timer += dt * 2.07808f;
			if(e->timer >= 4.18879f) e->step = 1;
			break;
		case 1:
			e->x = (float)cos(e->timer) * 135.0f + 128.0f;
			e->y = (float)sin(e->timer) * 135.0f + 172.0f;
			e->timer += dt * 0.942478f;
			if(e->timer >= 4.52040f) e->step = 2;
			break;
		case 2:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			if(e->y < 25.0f) { e->vel = 40.0f; e->step = 3; }
			break;
		case 3:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			if(e->x >= 256.0f) e->state = 0;
			break;
		}
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e);
		return;

	case ENEMY_POWERUP_RIGHT:
		switch(e->step) {
		case 0:
			e->x = (float)cos(e->timer) * 135.0f + 128.0f;
			e->y = (float)sin(e->timer) * 135.0f + 172.0f;
			e->timer -= dt * 2.07808f;
			if(e->timer <= -1.047197f) e->step = 1;
			break;
		case 1:
			e->x = (float)cos(e->timer) * 135.0f + 128.0f;
			e->y = (float)sin(e->timer) * 135.0f + 172.0f;
			e->timer -= dt * 0.942478f;
			if(e->timer <= -1.37881f) e->step = 2;
			break;
		case 2:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			if(e->y < 25.0f) { e->vel = 40.0f; e->step = 3; }
			break;
		case 3:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			if(e->x <= -16.0f) e->state = 0;
			break;
		}
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e);
		return;

	case ENEMY_FALLING_SHIFT:
		switch(e->step) {
		case 0:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			draw_frame(e->x, e->y, &e->fr[0]);
			if(e->y > 133.0f) {
				towards_player(&e->vx, &e->vy, e, 0);
				e->timer = 0;
				e->step = 1;
				e->vy = 0.866f;
				e->vx = e->vx > 0? 0.5f : -0.5f;
			}
			break;
		case 1:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			if(e->vx<0) draw_frame(e->x, e->y, &e->fr[1]);
			else       draw_framex(e->x, e->y, &e->fr[1], 1, 0);
			e->timer += dt;
			if(e->timer > 0.1333333) e->step = 2;
			break;
		case 2:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			if(e->vx<0) draw_frame(e->x, e->y, &e->fr[2]);
			else       draw_framex(e->x, e->y, &e->fr[2], 1, 0);
			if(e->y>240.0f) e->state = 0;
			break;
		}
		return;

	case ENEMY_TWISTER:
		e->x += (float)(e->vx * e->vel * dt);
		e->y += (float)(e->vy * e->vel * dt);

		if(e->timer >= 0)
		switch((int)(e->timer*4)) {
			case 0: draw_frame(e->x, e->y, &e->fr[0]); e->vel = 112 + e->step*8.0f; break;
			case 1: draw_frame(e->x, e->y, &e->fr[1]); e->vel = 112 + e->step*8.0f; break;
			case 2: draw_frame(e->x, e->y, &e->fr[2]); e->vel =  96 + e->step*8.0f; break;
			default: e->vel = 80; if(e->step != 0) e->state = 0;
			switch((int)(e->timer*30)&3) {
				case 0: draw_framex(e->x, e->y, &e->fr[2],0,0); break;
				case 1: draw_framex(e->x, e->y, &e->fr[3],1,1); break;
				case 2: draw_framex(e->x, e->y, &e->fr[2],1,1); break;
				case 3: draw_framex(e->x, e->y, &e->fr[3],0,0); break;
			}
		}

		if(e->x < -24.0f || e->x > 256.0f || e->y < -24.0f || e->y > 240.0f ||
			fall_y - map_offset > 224.0f)
			e->state = 0;

		e->timer += dt;
		return;

	case ENEMY_SPHINX:
		e->x = (float)cos(e->timer) * 27.0f + e->vx;
		e->y = (float)sin(e->timer) * 27.0f + e->vy;
		switch(e->step) {
		case 0:
			if(e->timer > -1.5707963) draw_frame(e->x, e->y, &e->fr[0]);
			if(e->vel > 2.0f) e->step++;
			break;
		case 1:
			draw_frame(e->x, e->y, &e->fr[0]);
			e->vy += (float)(70.0 * dt);
			if(e->vy > 120.0f) { e->step++; e->vel = 0; }
			break;
		case 2:
			draw_frame(e->x, e->y, &e->fr[0]);
			e->vx -= (float)((sin(e->vel* PI * 1.2) + 1.0) * 25.0 * dt);
			e->vy = (float)(sin(e->vel * PI * 0.6) * 60.5) + 120.0f;
			if(e->vx < 96.0f && e->vy <= 120.0f) e->step++;
			break;
		case 3:
			draw_frame(e->x, e->y, &e->fr[0]);
			e->vy -= (float)(70.0 * dt);
			if(e->vy < 48.0f) e->step++;
			break;
		case 4:
			draw_frame(e->x, e->y, &e->fr[0]);
			e->vx += (float)(60.0 * dt);
			if(e->vx > 116.0f) e->step++;
			break;
		case 5:
			draw_frame(e->x, e->y, &e->fr[0]);
			e->vy += (float)(60.0 * dt);
			if(e->y > 256.0f) e->state = 0;
			break;
		}
		e->vel += (float)dt;
		e->timer += 2.3271 * dt;
		shoot_ebullet(e);
		return;

	case ENEMY_SPHINX_BROKEN:
		e->x = (float)cos(e->timer) * 27.0f + e->vx;
		e->y = (float)sin(e->timer) * 27.0f + e->vy;
		switch(e->step) {
		case 0:
			if(e->timer > -1.5707963) draw_frame(e->x, e->y, &e->fr[1]);
			if(e->vel > 2.0f) e->step++;
			break;
		case 1:
			draw_frame(e->x, e->y, &e->fr[1]);
			e->vy += (float)(70.0 * dt);
			if(e->vy > 120.0f) { e->step++; e->vel = 0; }
			break;
		case 2:
			draw_frame(e->x, e->y, &e->fr[1]);
			e->vx -= (float)((sin(e->vel* PI * 1.2) + 1.0) * 25.0 * dt);
			e->vy = (float)(sin(e->vel * PI * 0.6) * 60.5) + 120.0f;
			if(e->vx < 96.0f && e->vy <= 120.0f) e->step++;
			break;
		case 3:
			draw_frame(e->x, e->y, &e->fr[1]);
			e->vy -= (float)(70.0 * dt);
			if(e->vy < 48.0f) e->step++;
			break;
		case 4:
			draw_frame(e->x, e->y, &e->fr[1]);
			e->vx += (float)(60.0 * dt);
			if(e->vx > 116.0f) e->step++;
			break;
		case 5:
			draw_frame(e->x, e->y, &e->fr[1]);
			e->vy += (float)(60.0 * dt);
			if(e->y > 256.0f) e->state = 0;
			break;
		}
		e->vel += (float)dt;
		e->timer += 2.3271 * dt;
		shoot_ebullet(e);
		return;

	case ENEMY_GRAYHEAD_BIG:
		e->x += (float)(e->vx * e->vel * dt);
		e->y += (float)(e->vy * e->vel * dt);
		switch(e->step) {
		case 0:
			draw_frame(e->x, e->y, &e->fr[0]);
			e->timer += dt;
			if(e->timer > 0.16666666) {
				e->x -= 4.0f;
				e->y -= 4.0f;
				e->w = 24;
				e->h = 24;
				e->step = 1;
			}
			break;
		case 1:
			draw_frame(e->x, e->y, &e->fr[1]);
			break;
		}
		if(e->x < -24.0f || e->x > 256.0f || e->y < -24.0f || e->y > 240.0f)
			e->state = 0;
		return;

	case ENEMY_BLUE_THINGY:
		switch(e->step) {
		case 0:
			e->x = (float)cos(time_accum * 3.0 * PI) * 4.0f + e->vx;
			e->y = (float)sin(time_accum * 6.0 * PI) * 3.0f + e->vy;
			e->vy += (float)(30.0 * dt);
			draw_frame(e->x, e->y, &e->fr[0]);
			if(e->vy > 80) {
				towards_player(&e->vx, &e->vy, e, 0);
				e->timer = 0;
				e->step = 1;
				e->vel = 140.0f;
			}
			break;
		case 1:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
			sprite_frame_adv(dt, e);
			if((int)e->time_cur >= 3) e->step = 2;
			break;
		case 2:
			e->x += (float)(e->vx * e->vel * dt);
			e->y += (float)(e->vy * e->vel * dt);
			draw_frame(e->x, e->y, &e->fr[3]);
			if(e->x < -24.0f || e->x > 256.0f || e->y < -24.0f || e->y > 240.0f)
				e->state = 0;
			break;
		}
		return;

	case ENEMY_DOWN_UP:
		switch(e->step) {
		case 0:
			e->y += (float)(e->vy * e->vel * dt);
			draw_frame(e->x, e->y, &e->fr[0]);
			if(e->y > e->vx) {
				e->step = 1;
				e->vx = e->x < 128.0f ? 1.0f : -1.0f;
				e->vy = e->y;
				e->vel = 70.0f;
				e->timer = 0;
			}
			break;
		case 1:
			e->x += (float)(e->vx * e->vel * dt);
			e->y  = (float)sin(e->timer * PI * 2.5f) * 14.0f + e->vy;
			draw_frame(e->x, e->y, &e->fr[e->timer < 0.2 ? 1 : 2]);
			e->timer += dt;
			if(e->timer >= 0.4) {
				e->step = 2;
				e->vy = -1;
				e->vel = 140.0f;
			}
			break;
		case 2:
			e->y += (float)(e->vy * e->vel * dt);
			draw_framex(e->x, e->y, &e->fr[0], 0, 1);
			if(e->y < -16.0f) e->state = 0;
			break;
		}
		shoot_ebullet(e);
		return;

	case ENEMY_TWISTY:
		e->y += (float)(e->vy * e->vel * dt);
		switch(e->step) {
		case 0:
			if(e->y > e->vx) {
				e->vx = e->x;
				e->timer = 0;
				e->step = 1;
				e->vel = 65.0f;
			}
			break;
		case 1:
			e->x = (float)sin(e->timer * PI) * 22.0f + e->vx;
			e->timer += dt;
			if(e->timer >= 1.0) { e->step = 2; e->vel = 80.0f; }
			break;
		}
		if(e->y > 240.0f) e->state = 0;
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e);
		return;

	case ENEMY_DIAGONAL:
		e->vel += (float)dt;
		switch(e->step) {
		case 0:
			if(e->x > 118.0f) {
				e->x -= (float)(124.0 * dt);
				if(e->x < e->vx) e->step = 1;
			} else {
				e->x += (float)(124.0 * dt);
				if(e->x > e->vx) e->step = 1;
			}
			break;
		case 1:
			shoot_ebullet(e);
			if(e->vel >= 2.25f) e->step = 2;
			break;
		case 2:
			e->y -= (float)(124 * dt);
			break;
		}
		if(e->y < -16.0f) e->state = 0;
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e);
		return;

	case ENEMY_FLOATY:
		e->y += (float)(e->vy * dt);
		if(e->y > 240.0f) e->state = 0;
		draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		sprite_frame_adv(dt, e);
		return;

	case ENEMY_DRAGON:
		switch(e->step)  {
		case 0:
			if(ground_enemy_count()) e->timer = time_accum + 2.0;
			if(time_accum > e->timer) { e->timer = 0; e->step++; }
			break;
		case 1:
			e->bullet_next = 1.6;
			e->vel = 42.3f;
			e->y += (float)(60 * dt);
			if(e->y >= 122 - 72) e->step++;
			break;
		case 2:
			e->x = 128 + (float)cos(e->timer) * 40 - 12 - 40;
			e->y = 122 + (float)sin(e->timer) * 40 - 72;
			e->timer += PI * 0.5 * dt;
			if(e->timer >= 2*PI) { e->timer -= 2*PI; e->step++; }
			shoot_dragon(e);
			e->bullet_next -= dt;
			e->vel -= (float)(dt * 10);
			if(e->vel < 0) { e->step += 2; e->vel = 0; };
			break;
		case 3:
			e->x = 128 - (float)cos(e->timer) * 40 - 12 + 40;
			e->y = 122 + (float)sin(e->timer) * 40 - 72;
			e->timer += PI * 0.5 * dt;
			if(e->timer >= 2*PI) { e->timer -= 2*PI; e->step--; }
			shoot_dragon(e);
			e->bullet_next -= dt;
			e->vel -= (float)(dt * 10);
			if(e->vel < 0) { e->step += 2; e->vel = 0; };
			break;
		case 4: case 5:
			if(e->vel < 4) {
				e->time_cur = 0;
				e->y += (float)(187.5 * dt); // 87
				e->vel += (float)(dt * 10);
				if(e->vel >= 4.0f) e->y += 12;
			} else {
				e->y -= (float)(60.0 * dt);
				e->vel += (float)(dt * 10);
				if(e->vel >= 18.5f) { e->step -= 2; e->vel = 60.5f; }
			}
			break;
		}

		if(e->vx > 0) {
			glEnable(GL_COLOR_LOGIC_OP);
			glLogicOp(GL_SET);
			draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
			glDisable(GL_COLOR_LOGIC_OP);
			e->vx -= (float)dt;
		} else {
			draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
		}

		sprite_frame_adv(dt, e);
		return;
	}
}

void boom_random_dragon(struct enemy *e)
{
	int num[3] = {-1,-1,-1};
	int n = -1;

	while(n == num[0]) n = rand()%10;                               num[0] = n;
	while(n == num[0] || n == num[1]) n = rand()%10;                num[1] = n;
	while(n == num[0] || n == num[1] || n == num[2]) n = rand()%10; num[2] = n;

	for(n = 0; n < 3; n++) {
		float x = (num[n]&1)  * 16.0f - 4.0f;
		float y = (num[n]>>1) * 16.0f;

		dragon[n].x = e->x + x;
		dragon[n].y = e->y + y;
	}
}

void boom_dragon(double dt, struct enemy *e)
{
	draw_frame(e->x, e->y, &e->fr[(int)e->time_cur]);
	sprite_frame_adv(dt, e);

	if(e->step < 10) {
		int i;

		for(i = 0; i < 3; i++)
			boom_animation(dt, &dragon[i]);

		if(dragon[0].state == ENEMY_END) {
			boom_random_dragon(e);
			play_sfx_pan(12, e->x+e->w*0.5f);
			e->step++;

			for(i = 0; i < 3; i++) {
				dragon[i].time_cur = 0;
				dragon[i].state = ENEMY_BOOM;
			}
		}
	} else {
		player1.bullet_next = 0;
		player2.bullet_next = 0;
		bg_flashing = 0;
		e->state    = 0;
		time_accum  = 0;
		game_scene  = SC_LEVEL1_END;
	}
}

void air_enemy_step(double dt)
{
	int i, z;

	for(i = 0; i < AIR_ENEMIES; i++) {
		switch(aenemy[i].state) {
		case ENEMY_NONE:
			continue;
		case ENEMY_NORMAL:
			hardcoded_step(dt, &aenemy[i]);
			continue;
		case ENEMY_STAY:
			hardcoded_stay(dt, &aenemy[i]);
			continue;
		case ENEMY_KILLED:
			if(aenemy[i].id == ENEMY_SPHINX) {
				aenemy[i].id = ENEMY_SPHINX_BROKEN;
				aenemy[i].hp = 10.0f;
				aenemy[i].state = ENEMY_NORMAL;
			} else if(aenemy[i].id == ENEMY_DRAGON) {
				int j;
				bg_flashing = 10.0;
				aenemy[i].state = ENEMY_BOOM;
				aenemy[i].step  = 0;
				for(j = 0; j < 3; j++) {
					dragon[j] = aenemy[i];
					dragon[j].time_cur = 0;
				}
			} else {
				aenemy[i].time_cur = 0;
				aenemy[i].step  = 0;
				aenemy[i].state = ENEMY_BOOM;
			}
			continue;
		case ENEMY_BOOM:
			if(aenemy[i].id == ENEMY_DRAGON)
				boom_dragon(dt, &aenemy[i]);
			else
				boom_animation(dt, &aenemy[i]);
			continue;
		case ENEMY_END:
			aenemy[i].state = 0;
			if(aenemy[i].id == ENEMY_POWERUP_LEFT ||
				aenemy[i].id == ENEMY_POWERUP_RIGHT) {
				for(z = 0; z < AIR_ENEMIES; z++) {
					if(!aenemy[z].state) {
						aenemy[z] = aenemy[i];
						aenemy[z].state = ENEMY_STAY;
						aenemy[z].id    = ENEMY_POWERUP_BONUS;
						aenemy[z].hp    = 0;
						break;
					}
			}}
			continue;
		}
	}
};

void player_add_life(struct player *p, int lives)
{
	p->lives += lives;
	if(p->lives > 9) p->lives = 9;
}

void player_add_score(struct player *p, int score)
{
	int old = p->score;
	int new = old + score;

	if((new/50000) > (old/50000)) {
		player_add_life(p, 1);
		play_sfx(3);
	}

	p->score += score;
	if(p->score > highscore) highscore = p->score;
}

void pbullets_collision(struct player *p, struct bullet *pbullet)
{
	float xl, xt, xr, xb;
	float yl, yt, yr, yb;
	int i, j;

	for(i = 0; i < PLAYER_BULLETS; i++) {
		if(!pbullet[i].state) continue;

		xl = pbullet[i].x;
		xt = pbullet[i].y;
		xr = xl + pbullet[i].w;
		xb = xt + pbullet[i].h;

		for(j = 0; j < AIR_ENEMIES; j++) {
			if(aenemy[j].state == ENEMY_NORMAL) {
				if(aenemy[j].id == ENEMY_TWISTER) continue;

				yl = aenemy[j].x;
				yt = aenemy[j].y;
				yr = yl + aenemy[j].w;
				yb = yt + aenemy[j].h;

				if(xl < yr && xr > yl && xt < yb && xb > yt) {
					int sfx_hit = 6;

					if(aenemy[j].score > 300 || pbullet[i].id == 0) {
						pbullet[i].state--;
						if(pbullet[i].state > 0) {
							pbullet[i].x = p->x - 8.0f;
							pbullet[i].y = p->y - 24.0f;
						} else {
							pbullet[i].state = 0;
						}
					}

					if(aenemy[j].id == ENEMY_DRAGON) {
						if(aenemy[j].step < 2) continue;
						aenemy[j].vx = 1.0f / 30.0f;
						sfx_hit = 12;
					}

					aenemy[j].hp -= 1.0f + pbullet[i].id;

					if(aenemy[j].hp <= 0) {
						aenemy[j].state = ENEMY_KILLED;
						play_sfx_pan(12, aenemy[j].x + aenemy[j].w * 0.5f);
						player_add_score(p, aenemy[i].score);
					} else {
						play_sfx_pan(sfx_hit, aenemy[j].x + aenemy[j].w * 0.5f);
					}
				}
			} else if(aenemy[j].state == ENEMY_STAY) {
				if(aenemy[j].id != ENEMY_BLOCKING_BIRD) continue;

				yl = aenemy[j].x;
				yt = aenemy[j].y;
				yr = yl + aenemy[j].w;
				yb = yt + aenemy[j].h;

				if(xl < yr && xr > yl && xt < yb && xb > yt) {
					pbullet[i].state = 0;
					play_sfx_pan(6, aenemy[j].x + aenemy[j].w * 0.5f);
				}

			}
		}
	}
}

void pbombs_collision(struct player *p, struct bullet *pbomb)
{
	float xl, xt, xr, xb;
	float yl, yt, yr, yb;
	int i;

	if(!pbomb->state) return;

	xl = pbomb->x;
	xt = pbomb->y;
	xr = xl + 8.0f;
	xb = xt + 8.0f;

	for(i = 0; i < GROUND_ENEMIES; i++) {
		if(genemy[i].state != ENEMY_NORMAL) continue;

		yl = genemy[i].x;
		yt = genemy[i].y;
		yr = yl + genemy[i].w;
		yb = yt + genemy[i].h;

		if(xl < yr && xr > yl && xt < yb && xb > yt) {
			pbomb->state = 0;
			genemy[i].hp--;

			if(genemy[i].hp <= 0) {
				genemy[i].state = ENEMY_KILLED;
				play_sfx_pan(12, genemy[i].x + genemy[i].w * 0.5f);
				player_add_score(p, genemy[i].score);
			} else {
				play_sfx_pan(6, genemy[i].x + genemy[i].w * 0.5f);
			}
		}
	}
}

void power_up(struct player *p)
{
	p->power++;
	if(p->power > 4) p->power = 4;
	play_sfx(4);
}

void player_hit(struct player *p)
{
	if(p->flicker > 0) return;

	p->power--;

	if(p->power < 0) {
		p->state = PLAYER_DEAD;
		p->power = 0;
		p->time_cur = 0;
		p->time_adv = 60.0f/5.0f;
		p->frames = 13.0f;
	} else {
		p->flicker = 3.0f;
	}

	play_sfx_pan(0, p->x);
}

int get_free_air_slots(int *buffer)
{
	int i, count = -1;

	for(i = 0; i < AIR_ENEMIES; i++) {
		if(!aenemy[i].state) {
			buffer[++count] = i;
		}
	}

	return count + 1;
}

void enemy_dragon(void)
{
	int i;
	for(i = 0; i < AIR_ENEMIES; i++) {
		if(!aenemy[i].state) {
			struct enemy *e = &aenemy[i];
			*e = enemyt[10];
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_DRAGON;
			e->hp = 160; // 32 * 5;
			e->x  = 128 - 12;
			e->y  =  -8 - 72;
			break;
		}
	}
}

void enemy_floaty(void)
{
	float pos[8][3] = {{24,0,120},{72,-32,60},{48,-128,120},{104,-192,120},
					{216,0,120},{168,-32,60},{192,-128,120},{152,-192,120}};

	int i, count = 0;
	for(i = 0; i < AIR_ENEMIES && count < 8; i++) {
		if(!aenemy[i].state) {
			struct enemy *e = &aenemy[i];
			*e = enemyt[9];
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_FLOATY;
			e->x = pos[count][0];
			e->y = pos[count][1];
			e->vy = pos[count][2];
			count++;
		}
	}
}

void enemy_diagonal(void) // 118
{
	float pos[6][3] = {{-20,145,47},{-85,113,73},{-150,81,94},
					    {256,145,190},{318,113,162},{380,81,142}};

	int i, count = 0;
	for(i = 0; i < AIR_ENEMIES && count < 6; i++) {
		if(!aenemy[i].state) {
			struct enemy *e = &aenemy[i];
			*e = enemyt[8];
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_DIAGONAL;
			e->x  = pos[count][0];
			e->y  = pos[count][1];
			e->vx = pos[count][2];
			e->vy = 0;
			e->vel = 0;
			e->hp = 3.0f;
			e->bullet_next = time_accum + 1.883333333;
			count++;
		}
	}
}

void enemy_twisty(int set)
{
	float pos[2][4][3] = {
		{{40,0,38},{104,-25,48},{11,-115,0},{8,-175,32}},
		{{104,0,16},{200,0,16},{104,-16,16},{144,-16,300}}};

	int i, count = 0;
	for(i = 0; i < AIR_ENEMIES && count < 4; i++) {
		if(!aenemy[i].state) {
			struct enemy *e = &aenemy[i];
			*e = enemyt[7];
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_TWISTY;
			e->x  = pos[set&1][count][0];
			e->y  = pos[set&1][count][1];
			e->vx = pos[set&1][count][2];
			e->vy = 1;
			e->vel = 80.0f;
			e->hp = 3.0f;
			count++;
		}
	}
}



void enemy_blocking_bird(float x, float y)
{
	int i;
	for(i = 0; i < AIR_ENEMIES; i++)
	if(!aenemy[i].state) {
		struct enemy *e = &aenemy[i];
		*e = enemyt[0];
		e->state = ENEMY_STAY;
		e->id   = ENEMY_BLOCKING_BIRD;
		e->step = 0;
		e->x = x;
		e->y = y;
		e->w = 32;
		e->h = 40;
		e->vel = 0;
		e->vx  = x;
		e->vy  = y;
		e->timer = 0;
		break;
	}
}

void enemy_down_up(int extra)
{
	float posx[8]  = { 40,  104,   8,   8, 104, 200, 106, 144};
	float endy[8]  = { 70,  157, 210,  73, 155, 200,  75, 214};
	float bnext[8] = {0.7f,9.0f,0.0f,1.5f,9.0f,9.0f,0.1f,9.0f};
	int i, count = 0;

	extra = (extra&1) + 7;

	for(i = 0; i < AIR_ENEMIES && count < extra; i++) {
		if(!aenemy[i].state) {
			struct enemy *e = &aenemy[i];
			*e = enemyt[6];
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_DOWN_UP;
			e->x = posx[count];
			e->y = count * -32.0f; //posy[count];
			e->vy = 1;
			e->vx = endy[count];
			e->vel = 124.0f;
			e->bullet_next = time_accum + bnext[count];
			count++;
		}
	}
}

void enemy_blue_thingy(float x, float y)
{
	int i;
	for(i = 0; i < AIR_ENEMIES; i++)
	if(!aenemy[i].state) {
		struct enemy *e = &aenemy[i];
		*e = enemyt[5];
		e->state = ENEMY_NORMAL;
		e->id    = ENEMY_BLUE_THINGY;
		e->hp = 4.0;
		e->vx = x;
		e->vy = y;
		break;
	}
}

void enemy_sphinx(void)
{
	int air_slot[AIR_ENEMIES], i;
	if(get_free_air_slots(air_slot) >= 4) {
		float posx[4] = {27, 0, -27, 0};
		float posy[4] = {0, 27, 0, -27};

		for(i = 0; i < 4; i++) {
			struct enemy *e = &aenemy[air_slot[i]];
			*e = enemyt[3];
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_SPHINX;
			e->vx = 200;
			e->vy = 28;
			e->hp = 10.0f;
			e->bullet_next = time_accum + 3.0;
			e->timer = -1.5707963f * (i+1);
		}
	}
}

void danger_entrance(void) {
	int air_slot[AIR_ENEMIES];
	if(get_free_air_slots(air_slot) >= 1) {
		struct enemy *e = &aenemy[air_slot[0]];
		*e = enemyt[0];
		e->state = ENEMY_STAY;
		e->id   = ENEMY_DANGER_DUMMY;
		e->step = 0;
		e->x = 128;
		e->y = 32;
		e->vel =  0;
		e->vx  = 0;
		e->vy  = 32;
		e->timer = 0;
	}
}

void enemy_twister(void)
{
	int air_slot[AIR_ENEMIES];
	int slots = get_free_air_slots(air_slot), i;

	if(fall_done) return;

	if(slots >= 4) {
		for(i = 0; i < 4; i++) {
			struct enemy *e = &aenemy[air_slot[--slots]];
			*e = enemyt[11];
			e->x   = fall_x;
			e->y   = fall_y - map_offset;
			e->vel = 0;
			e->state = ENEMY_NORMAL;
			e->id    = ENEMY_TWISTER;
			e->timer = -0.07142857 * i ;
			e->step  = i;
			towards_player(&e->vx, &e->vy, e, 0);
		}
	}
}

void enemy_falling_turn(int variation)
{
	int air_slot[AIR_ENEMIES];
	if(get_free_air_slots(air_slot) >= 4) {
		struct enemy *e1 = &aenemy[air_slot[0]];
		struct enemy *e2 = &aenemy[air_slot[1]];
		struct enemy *e3 = &aenemy[air_slot[2]];
		struct enemy *e4 = &aenemy[air_slot[3]];
		*e1 = enemyt[2];
		*e2 = enemyt[2];
		*e3 = enemyt[2];
		*e4 = enemyt[2];

		e1->state = e2->state = e3->state = e4->state = ENEMY_NORMAL;
		e1->id = e2->id = e3->id = e4->id = ENEMY_FALLING_SHIFT;
		e1->vy = e2->vy = e3->vy = e4->vy = 1.0f;

		e1->y = e4->y = -16.0f;
		e2->y = e3->y = -32.0f;

		switch(variation) {
		case 0:
			e1->x =  24.0f;
			e2->x =  72.0f;
			e3->x = 168.0f;
			e4->x = 216.0f;
			e1->vel = e4->vel = 63.0f;
			e2->vel = e3->vel =127.0f;
			break;
		case 1:
			e1->x =  48.0f;
			e2->x = 104.0f;
			e3->x = 152.0f;
			e4->x = 192.0f;
			e1->vel = e4->vel =127.0f;
			e2->vel = e3->vel = 63.0f;
			break;
		case 2:
			e1->x =  48.0f;
			e1->vel = 127.0f;
			e2->state = e3->state = e4->state = 0;
			break;
		}
	}
}

void powerup_right(void) // 3
{
	int air_slot[AIR_ENEMIES];
	if(get_free_air_slots(air_slot) >= 1) {
		struct enemy *e = &aenemy[air_slot[0]];
		*e = enemyt[0];
		e->state = ENEMY_NORMAL;
		e->id   = ENEMY_POWERUP_RIGHT;
		e->step = 0;
		e->x = 0;
		e->y = 0;
		e->vel =  70.0f;
		e->vx  = -0.981627f;
		e->vy  = -0.190809f;
		e->timer = 0;
	}
}

void powerup_left(void) // 2
{
	int air_slot[AIR_ENEMIES];
	if(get_free_air_slots(air_slot) >= 1) {
		struct enemy *e = &aenemy[air_slot[0]];
		*e = enemyt[0];
		e->state = ENEMY_NORMAL;
		e->id   = ENEMY_POWERUP_LEFT;
		e->step = 0;
		e->x = 0;
		e->y = 0;
		e->vel =  70.0f;
		e->vx  =  0.981627f;
		e->vy  = -0.190809f;
		e->timer = 3.14159f;
	}
}

void enemy_spiral_left(void) // 1
{
	int air_slot[AIR_ENEMIES], i;
	if(get_free_air_slots(air_slot) >= 8) {
		for(i = 0; i < 8; i++) {
			struct enemy *e = &aenemy[air_slot[i]];
			*e = enemyt[1];
			e->state = ENEMY_NORMAL;
			e->id   = ENEMY_SPIRAL_LEFT;
			e->step = 0;
			e->x = (float)(0 - 7*i);
			e->y = (float)(0 - 32*i);
			e->vel = 280.0f;
			e->vx  =  0.2142857f;
			e->vy  =  0.9642857f;
			sprite_frame_adv(0.18198 * (7-i), e);
		}
	}
}

void enemy_spiral_right(void)  // 0
{
	int air_slot[AIR_ENEMIES], i;
	if(get_free_air_slots(air_slot) >= 8) {
		for(i = 0; i < 8; i++) {
			struct enemy *e = &aenemy[air_slot[i]];
			*e = enemyt[1];
			e->state = ENEMY_NORMAL;
			e->id   = ENEMY_SPIRAL_RIGHT;
			e->step = 0;
			e->x = (float)(240 + 7*i);
			e->y = (float)(0 - 32*i);
			e->vel = 280.0f;
			e->vx  = -0.2142857f;
			e->vy  =  0.9642857f;
			sprite_frame_adv(0.18198 * (7-i), e);
		}
	}
}

void player_step(double dt, struct player *p, struct pinput *in,
	struct bullet *pbullet, struct bullet *pbomb)
{
	if(p->state == PLAYER_NORMAL) {
		p->x += in->x * (float)(dt * 128.0);
		p->y += in->y * (float)(dt * 128.0);

		p->x = clampf(p->x, 8.0f, 248.0f);
		p->y = clampf(p->y, 0.0f, 208.0f);

		if(in->buttons & INPUT_B) shoot_pgun(p, pbullet);
		if(in->buttons & INPUT_A) shoot_pbomb(p, pbomb);
	} else if(p->state == PLAYER_OVER) {
		p->flicker -= (float)dt;
	}
}

float absf(float a)
{
	return a < 0 ? -a : a;
}

void pvenemies_collision(struct player *p)
{
	float xl, xt, xr, xb;
	float yl, yt, yr, yb;
	int i;

	if(p->state != PLAYER_NORMAL) return;

	xl = p->x -  4.0f;
	xt = p->y + 10.0f;
	xr = xl + 8.0f;
	xb = xt + 8.0f;

	for(i = 0; i < AIR_ENEMIES; i++) {
		if(aenemy[i].state == ENEMY_NORMAL) {
			if(p->flicker > 0) continue;

			yl = aenemy[i].x;
			yt = aenemy[i].y;
			yr = yl + aenemy[i].w;
			yb = yt + aenemy[i].h;

			if(xl < yr && xr > yl && xt < yb && xb > yt) {
				if(aenemy[i].id == ENEMY_TWISTER) {
					game_scene = map_offset > 2880 ? SC_LEVEL1_DANGER : SC_LEVEL1_LUCKY;
					fall_done = 1;
					play_sfx(7);
				} else {
					aenemy[i].hp -= 1.0f;
					if(aenemy[i].hp <= 0) {
						aenemy[i].state = ENEMY_KILLED;
						play_sfx_pan(12, aenemy[i].x + aenemy[i].w * 0.5f);
					}
					player_hit(p);
				}
			}
		} else if(aenemy[i].state == ENEMY_STAY){
			yl = aenemy[i].x;
			yt = aenemy[i].y;
			yr = yl + aenemy[i].w;
			yb = yt + aenemy[i].h;

			if(xl < yr && xr > yl && xt < yb && xb > yt) {
				if(aenemy[i].id == ENEMY_POWERUP_BONUS) {
					aenemy[i].hp = 0;
					aenemy[i].state = ENEMY_NONE;
					power_up(p);
				} else if(aenemy[i].id == ENEMY_BLOCKING_BIRD) {
					float dl = absf(xr-yl);
					float dt = absf(xb-yt);
					float dr = absf(xl-yr);
					float db = absf(xt-yb);
					float dh = dl < dr ? dl : dr;
					float dv = dt < db ? dt : db;

					if(dh < dv) p->x += dl < dr ? -dl : dr;
					else        p->y += dt < db ? -dt : db;
				}
			}
		}
	}
}

void pvebullets_collision(struct player *p)
{
	float xl, xt, xr, xb;
	float yl, yt, yr, yb;
	int i;

	if(p->state != PLAYER_NORMAL || p->flicker > 0) return;

	xl = p->x -  4.0f;
	xt = p->y + 10.0f;
	xr = xl + 8.0f;
	xb = xt + 8.0f;

	for(i = 0; i < ENEMY_BULLETS; i++) {
		if(!ebullet[i].state) continue;

		yl = ebullet[i].x;
		yt = ebullet[i].y;
		yr = yl + ebullet[i].w;
		yb = yt + ebullet[i].h;

		if(xl < yr && xr > yl && xt < yb && xb > yt) {
			ebullet[i].state = 0;
			player_hit(p);
		}
	}
}

void level1_event(void)
{
	static int last_line;
	int cur_line = (int)map_offset >> 4;

	if(cur_line == last_line) return;

	switch(cur_line){
	case   0: enemy_dragon();
	case   1: play_music(boss_music); break;
	case   2: fadeout_music(level1_music, 1000); break;
	case  51: enemy_floaty(); break;
	case  60: enemy_floaty(); break;
	case  72: enemy_diagonal(); break;
	case  81: enemy_diagonal(); break;
	case  91: enemy_twisty(1); break;
	case  98: enemy_twisty(0); break;
	case 120: enemy_down_up(1); break;
	case 126: enemy_down_up(0); break;
	case 127: enemy_blue_thingy(150, -10); break;
	case 128: enemy_blue_thingy(105, -10); break;
	case 129: enemy_blue_thingy(192, -10); break;
	case 130: enemy_blue_thingy(46, -10); break;
	case 131: enemy_blue_thingy(170, -10); break;
	case 132: enemy_blue_thingy(154, -10);
			  enemy_blue_thingy(74, -10); break;
	case 133: enemy_blue_thingy(216, -10);
			  enemy_blue_thingy(104, -10); break;
	case 134: enemy_blue_thingy(192, -10);
			  enemy_blue_thingy(24, -10); break;
	case 135: enemy_blue_thingy(50, -10); break;
	case 136: enemy_blue_thingy(170, -10); break;
	case 137: enemy_blue_thingy(74, -10); break;
	case 138: enemy_blue_thingy(216, -10); break;
	case 139: enemy_blue_thingy(24, -10); break;
	case 182: enemy_spiral_left(); break;
	case 192: enemy_spiral_right(); break;
	case 217: enemy_sphinx(); break;
	case 221: enemy_twister(); break;
	case 223: enemy_twister(); break;
	case 225: enemy_twister(); break;
	case 226: enemy_twister(); break;
	case 228: enemy_twister(); break;
	case 230: enemy_twister(); play_sfx(11);
			  danger_entrance(); break;
	case 231: fall_x = 116; fall_y = 3736; fall_done = 0; break;
	case 238: enemy_falling_turn(2); break;
	case 240: enemy_falling_turn(0); break;
	case 244: enemy_falling_turn(1); break;
	case 246: enemy_falling_turn(0); break;
	case 250: enemy_falling_turn(1); break;
	case 253: enemy_falling_turn(0); break;
	case 255: enemy_falling_turn(1); break;
	case 258: enemy_falling_turn(0); break;
	case 261: enemy_spiral_left(); powerup_right(); break;
	case 270: enemy_spiral_right(); break;
	case 280: enemy_spiral_left(); break;
	case 284: powerup_left(); break;
	case 289: enemy_spiral_right(); break;
	case 299: enemy_spiral_left(); break;
	case 308: enemy_spiral_right(); break;
	}

	last_line = cur_line;
}

void draw_player_score(void)
{
	bind_texture(tex_atlas);
	draw_text_ui(24.0f, 216.0f, 0, "1P %d\n%08d", player1.lives, player1.score);
	if(two_players) {
		draw_text_ui(200.0f, 216.0f, 1, "2P %d", player2.lives);
		draw_text_ui(168.0f, 224.0f, 1, "%08d", player2.score);
	}
}

void scene_level1(double dt)
{
	level1_event();

	check_input();

	player_step(dt, &player1, &input[0], pbullet[0], &pbomb[0]);

	if(two_players)
		player_step(dt, &player2, &input[1], pbullet[1], &pbomb[1]);

	if(player1.state == PLAYER_OVER && player1.flicker <= 0) {
		if(!two_players)
			game_scene = SC_LEVEL1_OUT;
		else if(player2.state == PLAYER_OVER && player2.flicker <= 0)
			game_scene = SC_LEVEL1_OUT;
		if(game_scene == SC_LEVEL1_OUT)
			time_accum = 0;
	}

	bind_texture(tex_atlas);
	glColor3f(1, 1, 1);

	if(bg_flashing > 0 && (int)(bg_flashing*8)&1) {
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_COPY_INVERTED);
	}

	glPushMatrix();
		glTranslatef(0, -map_offset, 0);
		draw_map();
	glPopMatrix();

	bind_texture(tex_enemies);
	ground_enemy_step(dt);

	if(bg_flashing > 0) {
		glDisable(GL_COLOR_LOGIC_OP);
		bg_flashing -= dt;
	}

	air_enemy_step(dt);

	pbombs_collision(&player1, &pbomb[0]);
	pbullets_collision(&player1, pbullet[0]);
	pvenemies_collision(&player1);
	pvebullets_collision(&player1);

	if(two_players) {
		pbombs_collision(&player2, &pbomb[1]);
		pbullets_collision(&player2, pbullet[1]);
		pvenemies_collision(&player2);
		pvebullets_collision(&player2);
	}

	bind_texture(tex_bullets);
	enemy_bullets_step(dt);
	player_bombs_step(dt, &pbomb[0]);
	player_bullets_step(dt, &player1, pbullet[0]);
	if(two_players) {
		player_bombs_step(dt, &pbomb[1]);
		player_bullets_step(dt, &player2, pbullet[1]);
	}

	bind_texture(tex_player);
	draw_player(dt, 0, &player1);
	if(two_players)
		draw_player(dt, 1, &player2);

	bind_texture(tex_atlas);
	draw_player_score();

	if(map_offset > 0) map_offset -= (float)(30.0f * dt);
}

float smooth_out(float a, float b, float t)
{
	t = 1.0f - t;
	t = 1.0f - t*t;
	return a + (b-a)*t;
}

void draw_smooth_fall(float time, struct player *p)
{
	float x,y;

	time = time < 0 ? 0 : time > 1 ? 1 : time;

	x = smooth_out(p->x - 16.0f, fall_x - 4.0f, time);
	y = smooth_out(p->y, fall_y - map_offset - 8.0f, time);

	switch((int)(time*4)) {
		case 0:  draw_frame(x, y, &p->fr[ 2]); break;
		case 1:  draw_frame(x, y, &p->fr[16]); break;
		default: draw_frame(x, y, &p->fr[17]); break;
	}
}

void draw_smooth_return(float time, struct player *p)
{
	float x,y;

	time = time < 0 ? 0 : time > 1 ? 1 : time;

	x = smooth_out(fall_x - 4.0f, p->x - 16.0f, time);
	y = smooth_out(fall_y - map_offset - 8.0f, p->y, time);

	switch((int)(time*4)) {
		case 0:  draw_frame(x, y, &p->fr[17]); break;
		case 1:  draw_frame(x, y, &p->fr[16]); break;
		default: draw_frame(x, y, &p->fr[ 2]); break;
	}
}

void scene_level1_warning(double dt)
{
	static double accum;
	static int music_step;

	bind_texture(tex_atlas);
	glColor3f(1, 1, 1);

	glPushMatrix();
		glTranslatef(0, -map_offset, 0);
		draw_map();
	glPopMatrix();

	bind_texture(tex_enemies);
	ground_enemy_step(dt);
	air_enemy_step(dt);

	bind_texture(tex_bullets);
	enemy_bullets_step(dt);
	player_bombs_step(dt, &pbomb[0]);
	player_bullets_step(dt, &player1, pbullet[0]);
	if(two_players) {
		player_bombs_step(dt, &pbomb[1]);
		player_bullets_step(dt, &player2, pbullet[1]);
	}

	if(accum < 1) {
		bind_texture(tex_player);
		draw_smooth_fall((float)accum, &player1);
		if(two_players) draw_smooth_fall((float)accum, &player2);
		draw_player_score();
		music_step = 0;
	} else {
		char msg_danger[] = "Danger!";
		char msg_lucky[]  = "Lucky!";

		float col_black[3] = {0,0,0};
		float col_white[3] = {1,1,1};
		float col_purple[3] = {0.71f, 0.11f, 0.48f};
		float col_weird[3] = {0.97f, 0.85f, 0.65f};
		float time = (float)(accum-1.0) * 5.0f;

		char *msg   = msg_danger;
		float out_offset = 225.0f * 16.0f;
		float *col1 = col_black;
		float *col2 = col_purple;
		int snd_fx  = 0;
		int power = 0;

		if(game_scene == SC_LEVEL1_LUCKY) {
			msg = msg_lucky;
			out_offset = 106.0f * 16.0f;
			col1 = col_weird;
			col2 = col_white;
			snd_fx = 4;
			power = 4;
		}

		player1.power = power;
		player1.flicker = 3.0f;

		if(two_players) {
			player2.power = power;
			player2.flicker = 3.0f;
		}

		bind_texture(tex_atlas);

		switch(music_step) {
		case 0: fadeout_music(level1_music, 1000); music_step++; break;  case 1: break;
		case 2: fadeout_music(warning_music, 1000); music_step++; break; case 3: break;
		case 4: play_sfx(snd_fx); music_step++; break;                   case 5: break;
		case 6: play_music(level1_music); music_step++; break;
		}

		switch((int)time) {
		case 0: player1.x = 128.0f - 24.0f;
				player1.y = 172.0f;
				if(two_players) { player2.x = player1.x + 48.0f; player2.y = player1.y; }
				draw_fade(time); break;
		case 1: draw_fade(1); break;
		case 2: draw_fade(1);
				draw_text(104, 104, msg);
				draw_fade(3.0f - time);
				play_music(warning_music); break;
		case 3: case 4: case 5: case 6: case 7: case 8: case 9:
				if((int)((time-3.0f)*3.75f)&1) draw_color_fill(col2, 1);
				else                           draw_color_fill(col1, 1);
				draw_text(104, 104, msg); break;
		case 10:draw_fade(1); if(music_step == 1) music_step = 2;
				draw_text(104, 104, msg);
				draw_fade(time - 10.0f); break;
		case 11: case 12: case 13:
				draw_fade(1); if(music_step == 3) music_step = 4;
				draw_text( 88, 104, "Not in demo"); break;
		case 14:draw_fade(1); map_offset = out_offset;
				draw_text( 88, 104, "Not in demo");
				draw_fade(time - 14.0f); break;
		case 15:draw_fade(16.0f - time); if(music_step == 5) music_step = 6; break;
		default:time = (time-16.0f)*0.4f;
				bind_texture(tex_player);
				draw_smooth_return(time,&player1);
				if(two_players) draw_smooth_return(time,&player2);
				draw_player_score();
				if(time >= 1.0f) game_scene = SC_LEVEL1;
		}
	}

	bind_texture(tex_atlas);
	accum += dt * 0.4;

	if(game_scene == SC_LEVEL1) {
		music_step = -1;
		accum = 0;
	}
}

int end_anim_player(float x, double dt, struct player *p)
{
	struct frame tile = { 0, 0, 32, 128, 64, 16, 256, 256};
	float dx, dy;

	bind_texture(tex_player);

	switch(p->state) {
	case 1:
		dx = x - p->x;
		dy = 116 - p->y;
		p->x += (float)(dx*dt);
		p->y += (float)(dy*dt);
		draw_player(dt, 0, p);
		if(absf(dx) < 1.0f && absf(dy) < 1.0f) p->state++;
		break;
	case 2:
		switch((int)p->bullet_next) {
		case 0: draw_frame(p->x - 16.0f, p->y, &p->fr[ 2]); break;
		case 1: draw_frame(p->x - 16.0f, p->y, &p->fr[16]); break;
		case 2: draw_frame(p->x - 16.0f, p->y, &p->fr[17]); break;
		case 3: draw_frame(p->x - 16.0f, p->y, &p->fr[18]); break;
		}
		p->bullet_next += dt*2.0;
		if(p->bullet_next >= 4.0) p->state++;
		break;
	case 3:
		p->y -= (float)(60*dt); // 5 frames
		draw_frame(p->x - 16.0f, p->y, &p->fr[18 + ((int)p->bullet_next&1)]);
		p->bullet_next += 12*dt;
		bind_texture(tex_atlas);
		draw_frame(96, 48 + map_offset, &tile);
		if(p->y < 48.0f) p->state++;
		break;
	case 4:
		time_accum = 0;
		p->state++;
	case 5:
		return 1;
	}

	return 0;
}

void scene_level1_end(double dt)
{
	int change_scene = 0;

	bind_texture(tex_atlas);
	glColor3f(1, 1, 1);

	glPushMatrix();
		glTranslatef(0, -map_offset, 0);
		draw_map();
	glPopMatrix();

	change_scene = end_anim_player(128-8, dt, &player1);

	if(two_players)
		change_scene &= end_anim_player(128+8, dt, &player2);

	draw_player_score();

	if(change_scene) {
		draw_fade_out(SC_VICTORY_IN);
		if(game_scene == SC_VICTORY_IN) {
			fadeout_music(level1_music,1000);
			fadeout_music(boss_music,1000);
		}
	}
}

void scene_level1_out(double dt)
{
	bind_texture(tex_atlas);
	glColor3f(1, 1, 1);

	glPushMatrix();
		glTranslatef(0, -map_offset, 0);
		draw_map();
	glPopMatrix();

	draw_player_score();

	if(map_offset > 0) map_offset -= (float)(30.0f * dt);

	draw_fade_out(SC_GAMEOVER_IN);
	if(game_scene == SC_GAMEOVER_IN) {
		fadeout_music(level1_music,1000);
		fadeout_music(boss_music,1000);
	}
}

void draw_gameover(void)
{
	draw_text(88, 112, "Game  Over");
}

void draw_highscore(void)
{
	draw_text( 56, 80, "HI Score"); draw_text_ui(136, 80, 2, "%08d", highscore);
	draw_text( 56, 96, "1 Player"); draw_text_ui(136, 96, 2, "%08d", player1.score);
	draw_text(136, 112, "Game Over");

	if(two_players) {
		draw_text( 56, 128, "2 Player"); draw_text_ui(136, 96, 2, "%08d", player2.score);
		draw_text(136, 144, "Game Over");
	}
}

void scene_gameover_in(double dt)
{
	bind_texture(tex_atlas);
	draw_fancy_border();
	draw_gameover();
	play_music(gameover_music);
	draw_fade_in(SC_GAMEOVER);
}

void scene_gameover(double dt)
{
	bind_texture(tex_atlas);
	draw_fancy_border();

	if(time_accum < 7.0) {
		glColor3f(1, 1, 1);
		draw_gameover();
		check_input();
		if(input[0].buttons & INPUT_START) time_accum = 7.0;
	} else if(time_accum < 7.5) {
		float val = 1.0f - (float)(time_accum - 7.0) * 2.0f;
		glColor3f(val, val, val);
		draw_gameover();
		glColor3f(1, 1, 1);
	} else if(time_accum < 8.0) {
		float val = (float)(time_accum - 7.5) * 2.0f;
		glColor3f(val, val, val);
		draw_highscore();
		glColor3f(1, 1, 1);
		BASS_ChannelStop(gameover_music);
		play_music(hiscore_music);
	} else if(time_accum < 21.0) {
		glColor3f(1, 1, 1);
		draw_highscore();
		check_input();
		if(input[0].buttons & INPUT_START) time_accum = 21.0;
	} else {
		BASS_ChannelStop(hiscore_music);
		game_scene = SC_GAMEOVER_OUT;
		time_accum = 0;
	}
}

void scene_gameover_out(double dt)
{
	bind_texture(tex_atlas);
	draw_fancy_border();
	draw_highscore();
	draw_fade_out(SC_START_IN);
}

void draw_victory_text(void)
{
	draw_text(48, 96, "The devil is waiting");
	draw_text(48,112, "for us in the palace.");
	draw_text(48,128, " Rush couragously.");
	draw_text(48,176, "   * End of Demo *");
}

void draw_victory_score(void)
{
	draw_text(96, 80, "HI Score");
	draw_text(96, 96, "%08d", highscore);
	draw_text(96,120, "Player 1");
	draw_text(96,136, "%08d", player1.score);

	if(two_players) {
		draw_text(96,152, "Player 2");
		draw_text(96,168, "%08d", player2.score);
	}
}

void scene_victory_in(double dt)
{
	bind_texture(tex_atlas);
	draw_fancy_border();
	draw_victory_text();
	play_music(victory_music);
	draw_fade_in(SC_VICTORY);
}

void scene_victory(double dt)
{
	bind_texture(tex_atlas);
	draw_fancy_border();

	if(time_accum < 10.0) {
		glColor3f(1, 1, 1);
		draw_victory_text();
		check_input();
		if(input[0].buttons & INPUT_START) time_accum = 10.0;
	} else if(time_accum < 10.5) {
		float val = 1.0f - (float)(time_accum - 10.0) * 2.0f;
		glColor3f(val, val, val);
		draw_victory_text();
		glColor3f(1, 1, 1);
	} else if(time_accum < 11.0) {
		float val = (float)(time_accum - 10.5) * 2.0f;
		glColor3f(val, val, val);
		draw_victory_score();
		glColor3f(1, 1, 1);
	} else if(time_accum < 17.0) {
		glColor3f(1, 1, 1);
		draw_victory_score();
		check_input();
		if(input[0].buttons & INPUT_START) time_accum = 21.0;
	} else {
		BASS_ChannelStop(victory_music);
		game_scene = SC_VICTORY_OUT;
		time_accum = 0;
	}
}

void scene_victory_out(double dt)
{
	bind_texture(tex_atlas);
	draw_fancy_border();
	draw_victory_score();
	draw_fade_out(SC_START_IN);
}

void lwings_game(double dt)
{
	time_accum += dt;

	switch(game_scene) {
	case SC_START_IN:  scene_start_in(dt);  break;
	case SC_START:     scene_start(dt);     break;
	case SC_START_OUT: scene_start_out(dt); break;
	case SC_LEVEL1_IN:    scene_level1_in(dt);    break;
	case SC_LEVEL1:       scene_level1(dt);       break;
	case SC_LEVEL1_DANGER:scene_level1_warning(dt);break;
	case SC_LEVEL1_LUCKY: scene_level1_warning(dt);break;
	case SC_LEVEL1_END:   scene_level1_end(dt);   break;
	case SC_LEVEL1_OUT:   scene_level1_out(dt);   break;
	case SC_VICTORY_IN:   scene_victory_in(dt);  break;
	case SC_VICTORY:      scene_victory(dt);     break;
	case SC_VICTORY_OUT:  scene_victory_out(dt); break;
	case SC_GAMEOVER_IN:  scene_gameover_in(dt);  break;
	case SC_GAMEOVER:     scene_gameover(dt);     break;
	case SC_GAMEOVER_OUT: scene_gameover_out(dt); break;
	}
}
