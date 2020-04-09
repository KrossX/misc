/* Copyright (c) 2018 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "lwings.h"

extern GLuint tex_atlas, tex_bullets, tex_enemies, tex_player;

static char txtbuf[1024];

void sprite_frame_adv(double dt, void *spr)
{
	struct sprite *s = spr;
	s->time_cur += (float)(s->time_adv * dt);
	if(s->time_cur >= s->frames) s->time_cur -= s->frames;
}

void draw_quad_wire(float x, float y, float w, float h)
{
	float px1 = x;
	float py1 = y;
	float px2 = x + w;
	float py2 = y + h;

	GLfloat pos[4][2] = {{px1, py1}, {px1, py2}, {px2, py1}, {px2, py2}};

	GLboolean eglt2d = glIsEnabled(GL_TEXTURE_2D);
	if(eglt2d) glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINE_STRIP);
		glVertex2fv(pos[0]);
		glVertex2fv(pos[1]);
		glVertex2fv(pos[3]);
		glVertex2fv(pos[2]);
		glVertex2fv(pos[0]);
	glEnd();

	if(eglt2d) glEnable(GL_TEXTURE_2D);
}

void draw_quad(float x, float y, float w, float h)
{
	float px1 = x;
	float py1 = y;
	float px2 = x + w;
	float py2 = y + h;

	GLfloat pos[4][2] = {{px1, py1}, {px1, py2}, {px2, py1}, {px2, py2}};

	GLboolean eglt2d = glIsEnabled(GL_TEXTURE_2D);
	if(eglt2d) glDisable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLE_STRIP);
		glVertex2fv(pos[0]);
		glVertex2fv(pos[1]);
		glVertex2fv(pos[2]);
		glVertex2fv(pos[3]);
	glEnd();

	if(eglt2d) glEnable(GL_TEXTURE_2D);
}

void draw_frame(float x, float y, struct frame *f)
{
	float tx1 = f->u / f->tw;
	float ty1 = f->v / f->th;
	float tx2 = (f->u + f->w) / f->tw;
	float ty2 = (f->v + f->h) / f->th;

	float px1 = x + f->ox;
	float py1 = y + f->oy;
	float px2 = px1 + f->w;
	float py2 = py1 + f->h;

	GLfloat pos[4][2] = {{px1, py1}, {px1, py2}, {px2, py1}, {px2, py2}};
	GLfloat tex[4][2] = {{tx1, ty1}, {tx1, ty2}, {tx2, ty1}, {tx2, ty2}};

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2fv(tex[0]); glVertex2fv(pos[0]);
		glTexCoord2fv(tex[1]); glVertex2fv(pos[1]);
		glTexCoord2fv(tex[2]); glVertex2fv(pos[2]);
		glTexCoord2fv(tex[3]); glVertex2fv(pos[3]);
	glEnd();
}

void draw_framex(float x, float y, struct frame *f, int flipx, int flipy)
{
	float tx1 = flipx ? (f->u + f->w) / f->tw : f->u / f->tw;
	float ty1 = flipy ? (f->v + f->h) / f->th : f->v / f->th;
	float tx2 = flipx ? f->u / f->tw : (f->u + f->w) / f->tw;
	float ty2 = flipy ? f->v / f->th : (f->v + f->h) / f->th;

	float px1 = x + f->ox;
	float py1 = y + f->oy;
	float px2 = px1 + f->w;
	float py2 = py1 + f->h;

	GLfloat pos[4][2] = {{px1, py1}, {px1, py2}, {px2, py1}, {px2, py2}};
	GLfloat tex[4][2] = {{tx1, ty1}, {tx1, ty2}, {tx2, ty1}, {tx2, ty2}};

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2fv(tex[0]); glVertex2fv(pos[0]);
		glTexCoord2fv(tex[1]); glVertex2fv(pos[1]);
		glTexCoord2fv(tex[2]); glVertex2fv(pos[2]);
		glTexCoord2fv(tex[3]); glVertex2fv(pos[3]);
	glEnd();
}

void draw_frame_mirrorx(float x, float y, struct frame *f)
{
	float tx2 = f->u / f->tw;
	float ty1 = f->v / f->th;
	float tx1 = (f->u + f->w) / f->tw;
	float ty2 = (f->v + f->h) / f->th;

	float px1 = x;
	float py1 = y;
	float px2 = px1 + f->w;
	float py2 = py1 + f->h;

	GLfloat pos[4][2] = {{px1, py1}, {px1, py2}, {px2, py1}, {px2, py2}};
	GLfloat tex[4][2] = {{tx1, ty1}, {tx1, ty2}, {tx2, ty1}, {tx2, ty2}};

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2fv(tex[0]); glVertex2fv(pos[0]);
		glTexCoord2fv(tex[1]); glVertex2fv(pos[1]);
		glTexCoord2fv(tex[2]); glVertex2fv(pos[2]);
		glTexCoord2fv(tex[3]); glVertex2fv(pos[3]);
	glEnd();
}


void draw_text_ui(float x, float y, int id, char *fmt, ...)
{
	struct frame cf = {0, 0, 0, 144.0f + id * 8.0f, 8, 8, 256, 256};
	char *str = txtbuf;
	float x0 = x;

	va_list args;
	va_start(args, fmt);
	wvsprintf(txtbuf, fmt, args);
	va_end(args);

	while(*str) {
		int c = *str++;

		if(c >= '0' && c <= '9') c -= '0';
		else if(c == 'P') c = 10;
		else if(c == 'H') c = 11;
		else if(c == 'I') c = 12;
		else if(c == 'p') c = 10;
		else if(c == 'h') c = 11;
		else if(c == 'i') c = 12;
		else {
			if(c == '\n') { y += 8.0f; x = x0 - 8.0f; }
			c = -1;
		}

		if(c >= 0) {
			cf.u = c * 8.0f;
			draw_frame(x, y, &cf);
		}

		x += 8.0f;
	}
}

void draw_text(float x, float y, char *fmt, ...)
{
	struct frame cf = { 0, 0, 0, 0, 8, 8, 256, 256};
	char *str = txtbuf;
	float x0 = x;

	va_list args;
	va_start(args, fmt);
	wvsprintf(txtbuf, fmt, args);
	va_end(args);

	while(*str) {
		int c = *str++;

		if(c >= '0' && c <= '9') c -= '0';
		else if(c >= 'A' && c <= 'Z') c = c - 'A' + 10;
		else if(c >= 'a' && c <= 'z') c = c - 'a' + 10;
		else if(c == '@')  c = 36;
		else if(c == ',')  c = 37;
		else if(c == '.')  c = 38;
		else if(c == '>')  c = 39;
		else if(c == '<')  c = 40;
		else if(c == '!')  c = 41;
		else if(c == '*')  c = 42;
		else if(c == '\'') c = 44;
		else if(c == '-')  c = 45;
		else {
			if(c == '\n') { y += 8.0f; x = x0 - 8.0f; }
			c = -1;
		}

		if(c >= 0) {
			cf.u = (float)((c & 0xF) << 3) + 128.0f;
			cf.v = (float)((c & ~0xF) >> 1) + 232.0f;
			draw_frame(x, y, &cf);
		}

		x += 8.0f;
	}
}

void draw_color_fill(float *col3, float alpha)
{
	GLfloat pos[4][2] = {{0, 0}, {0, 240.0f}, {256.0f, 0}, {256.0f, 240.0f}};

	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glColor4f(col3[0], col3[1], col3[2], alpha);

	glBegin(GL_TRIANGLE_STRIP);
		glVertex2fv(pos[0]);
		glVertex2fv(pos[1]);
		glVertex2fv(pos[2]);
		glVertex2fv(pos[3]);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glColor3f(1, 1, 1);
}

void draw_fade(float alpha)
{
	GLfloat pos[4][2] = {{0, 0}, {0, 240.0f}, {256.0f, 0}, {256.0f, 240.0f}};

	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glColor4f(0, 0, 0, alpha);

	glBegin(GL_TRIANGLE_STRIP);
		glVertex2fv(pos[0]);
		glVertex2fv(pos[1]);
		glVertex2fv(pos[2]);
		glVertex2fv(pos[3]);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glColor3f(1, 1, 1);
}

void draw_fancy_border(void)
{
	struct frame wing_big    = {0, 0,  16, 208, 32, 48, 256, 256};
	struct frame wing_small  = {0, 0, 224, 128, 32, 32, 256, 256};
	struct frame swirl_big   = {0, 0,   0, 208, 16, 48, 256, 256};
	struct frame swirl_small = {0, 0, 208, 144, 16, 16, 256, 256};
	int i;

	glColor3f(0, 0, 0);
	glDisable(GL_TEXTURE_2D);
	draw_quad(0, 0, 256, 240);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);

	draw_frame(0.0f, 16.0f, &wing_big);
	draw_framex(224.0f, 16.0f, &wing_big, 1, 0);

	for(i = 0; i < 12; i++) {
		draw_frame(32.0f + i * 16.0f, 48.0f, &swirl_small);
		draw_frame(32.0f + i * 16.0f,192.0f, &swirl_small);
	}

	draw_frame(112.0f,  32.0f, &wing_small);
	draw_frame(  0.0f, 176.0f, &wing_small);
	draw_frame(224.0f, 176.0f, &wing_small);

	draw_frame(16.0f, 64.0f, &swirl_big);
	draw_frame( 0.0f, 96.0f, &swirl_big);
	draw_frame(16.0f,128.0f, &swirl_big);

	draw_frame(224.0f, 64.0f, &swirl_big);
	draw_frame(240.0f, 96.0f, &swirl_big);
	draw_frame(224.0f,128.0f, &swirl_big);
}

void draw_fade_in(int next)
{
	extern int game_scene;
	extern double time_accum;

	draw_fade(1.0f - (float)time_accum * 2.0f);
	if(time_accum >= 0.5) {
		game_scene = next;
		time_accum = 0;
	}
}

void draw_fade_out(int next)
{
	extern int game_scene;
	extern double time_accum;

	draw_fade((float)time_accum * 2.0f);
	if(time_accum >= 0.5) {
		game_scene = next;
		time_accum = 0;
	}
}

void dead_animation(int id, struct player *p)
{
	struct frame fr = {0, 0, 96, 0, 32, 32, 256, 64};
	int step = (int)p->time_cur;

	switch(step) {
	case 0: draw_frame(p->x - 16.0f, p->y, &p->fr[2]); return;
	case 1: draw_frame(p->x - 16.0f, p->y, &p->fr[3]); return;
	case 2: draw_frame(p->x - 16.0f, p->y, &p->fr[2]); return;
	case 3: draw_frame(p->x - 16.0f, p->y, &p->fr[3]); return;
	case 4: draw_frame(p->x - 16.0f, p->y, &p->fr[2]); return;
	case 5: draw_frame(p->x - 16.0f, p->y, &p->fr[3]); return;

	case 6:
		draw_frame(p->x - 16.0f, p->y, &p->fr[2]);
		fr.h = 24;
		draw_frame(p->x - 16.0f, p->y, &fr);
		return;

	case 7:
		draw_frame(p->x - 16.0f, p->y, &p->fr[2]);
		fr.v = 0;
		fr.h = 16;
		draw_frame(p->x - 16.0f, p->y, &fr);
		fr.v = 24;
		fr.h = 8;
		draw_frame(p->x - 16.0f, p->y + 24.0f, &fr);
		return;

	case 8:
		draw_frame(p->x - 16.0f, p->y, &p->fr[2]);
		fr.v = 0;
		fr.h = 8;
		draw_frame(p->x - 16.0f, p->y, &fr);
		fr.v = 16;
		fr.h = 16;
		draw_frame(p->x - 16.0f, p->y + 16.0f, &fr);
		return;

	case 9:
		draw_frame(p->x - 16.0f, p->y, &p->fr[2]);
		fr.v = 8;
		fr.h = 24;
		draw_frame(p->x - 16.0f, p->y + 8.0f, &fr);
		return;

	case 10:
		fr.u = 128;
		fr.w = 8;
		fr.h = 32;
		draw_frame(p->x - 4.0f, p->y, &fr);
		return;

	case 11:
		fr.u = 128;
		fr.w = 8;
		fr.h = 16;
		draw_frame(p->x - 4.0f, p->y + 8.0f, &fr);
		return;

	case 12:
		if(p->lives > 0) {
			extern int two_players;
			p->lives--;
			p->x = two_players ? 112.0f + 32.0f * id : 128.0f;
			p->y = 240.0f;
			p->time_adv = 60.0f / 9.0f;
			p->time_cur = 0;
			p->frames  = 3.0f;
			p->flicker = 3.0f;
			p->state   = PLAYER_SPAWN;
		} else {
			p->state = PLAYER_OVER;
			p->flicker = 5.0f;
		}
		return;
	}
}

void draw_player(double dt, int id, struct player *p)
{
	int flicker_draw = 1;

	if(p->flicker > 0) {
		flicker_draw = (int)(p->flicker * 30.0f)&1;
		p->flicker -= (float)dt;
	}

	switch(p->state) {
	case PLAYER_SPAWN:
		p->y -= (float)(dt * 100.0f);
		if(p->y <= 158.0f) p->state = PLAYER_NORMAL;
		if(flicker_draw) draw_frame(p->x - 16.0f, p->y, &p->fr[(int)p->time_cur]);
		break;
	case PLAYER_NORMAL: {
		extern struct frame playerf[16];
		struct frame *fr = p->power == 4 ? &playerf[8 + ((int)(p->time_cur*2.0)&1) * 4] : p->fr;
		if(flicker_draw) draw_frame(p->x - 16.0f, p->y, &fr[(int)p->time_cur]);
		} break;
	case PLAYER_DEAD:
		dead_animation(id, p);
		break;
	case PLAYER_OVER:
		return;
	}

	sprite_frame_adv(dt, p);
}

struct frame boomf[9] = {
	{4, 4, 184, 16,  8,  8, 256, 128},
	{0, 0, 184,  0, 16, 16, 256, 128},
	{0, 0, 200,  0, 16, 16, 256, 128},
	{0, 0, 216,  0, 16, 16, 256, 128},
	{0, 0, 200,  0, 16, 16, 256, 128},
	{0, 0, 216,  0, 16, 16, 256, 128},
	{0, 0, 232,  0, 16, 16, 256, 128},
	{4, 4, 248,  0,  8,  8, 256, 128},
	{4, 4, 248,  8,  8,  8, 256, 128}};

void boom_animation(double dt, struct enemy *e)
{
	int step = (int)e->time_cur;
	float time_adv = 60.0f / 3.0f;

	switch(step) {
		case 0: draw_frame (e->x, e->y, &boomf[0]); time_adv = 60.0f; break;
		case 1: draw_frame (e->x, e->y, &boomf[1]); break;
		case 2: draw_frame (e->x, e->y, &boomf[2]); break;
		case 3: draw_frame (e->x, e->y, &boomf[3]); break;
		case 4: draw_framex(e->x, e->y, &boomf[4], 1, 1); break;
		case 5: draw_framex(e->x, e->y, &boomf[5], 1, 1); break;
		case 6: draw_frame (e->x, e->y, &boomf[6]); break;
		case 7: draw_frame (e->x, e->y, &boomf[7]); break;
		case 8: draw_frame (e->x, e->y, &boomf[8]); break;
		case 9: e->state = ENEMY_END; return;
	}

	e->time_cur += (float)(time_adv * dt);
}
