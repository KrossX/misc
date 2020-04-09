/* Copyright (c) 2018 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "lwings.h"
#include "lodepng_mini.h"
#include "bass.h"

//nVIDIA / AMD Performance Hints
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

static int wnd_width;
static int wnd_height;

static GLfloat wnd_offx;
static GLfloat wnd_offy;
static GLfloat wnd_scale;

static HDC   dev_ctx;
static HGLRC ren_ctx;

static int show_fps;
static int vsync;
static int int_scale;

typedef BOOL (WINAPI *WGLSWAPINTERVAL)(int interval);
WGLSWAPINTERVAL wglSwapInterval;

GLuint tex_atlas, tex_bullets, tex_enemies, tex_player, tex_fb;

HSTREAM gameover_music;
HSTREAM hiscore_music;
HSTREAM level1_music;
HSTREAM warning_music;
HSTREAM victory_music;
HSTREAM boss_music;
HSAMPLE sndfx[18];
HCHANNEL sndch[18];

//int snd_vol = 900;
int snd_vol = 4000; // MAX 10000

BYTE keyboard[256];

void toggle_fullscreen(HWND wnd)
{
	static WINDOWPLACEMENT prev = {sizeof(WINDOWPLACEMENT)};

	DWORD style = GetWindowLong(wnd, GWL_STYLE);

	if(style & WS_OVERLAPPEDWINDOW)	{
		int width  = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);

		if(GetWindowPlacement(wnd, &prev)) {
			ShowCursor(FALSE);
			SetWindowLong(wnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(wnd, HWND_TOP, 0, 0, width, height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	} else {
		ShowCursor(TRUE);
		SetWindowLong(wnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(wnd, &prev);
		SetWindowPos(wnd, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

void window_resize(int width, int height)
{
	int ox, oy, dw, dh;
	GLfloat sx, sy;

	wnd_width  = width;
	wnd_height = height;

	sx = (GLfloat)(width / 256.0f);
	sy = (GLfloat)(height / 240.0f);

	if(int_scale) {
		if(sx >= 1.0) sx = (int)sx * 1.0f;
		if(sy >= 1.0) sy = (int)sy * 1.0f;
	}

	wnd_scale = sx < sy ? sx : sy;

	dw = (int)(wnd_scale * 256.0f);
	dh = (int)(wnd_scale * 240.0f);

	ox = (width - dw) / 2;
	oy = (height - dh) / 2;

	wnd_offx = (GLfloat)ox;
	wnd_offy = (GLfloat)oy;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -1, 100);
	glMatrixMode(GL_MODELVIEW);

	glScissor(ox, oy, dw, dh);
}

int set_pixel_format(void)
{
	PIXELFORMATDESCRIPTOR pfd;
	int pf;

	pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion        = 1;
    pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType      = PFD_TYPE_RGBA;
    pfd.cColorBits      = 32;
    pfd.cRedBits        = 0;
    pfd.cRedShift       = 0;
    pfd.cGreenBits      = 0;
    pfd.cGreenShift     = 0;
    pfd.cBlueBits       = 0;
    pfd.cBlueShift      = 0;
    pfd.cAlphaBits      = 0;
    pfd.cAlphaShift     = 0;
    pfd.cAccumBits      = 0;
    pfd.cAccumRedBits   = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits  = 0;
    pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits      = 0;
    pfd.cStencilBits    = 0;
    pfd.cAuxBuffers     = 0;
    pfd.iLayerType      = PFD_MAIN_PLANE;
    pfd.bReserved       = 0;
    pfd.dwLayerMask     = 0;
    pfd.dwVisibleMask   = 0;
    pfd.dwDamageMask    = 0;

	pf = ChoosePixelFormat(dev_ctx, &pfd);
	if(!pf) return 0;

	return SetPixelFormat(dev_ctx, pf, &pfd);
}

LRESULT CALLBACK wndproc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
		case WM_CREATE:
			dev_ctx = GetDC(wnd);

			if(!set_pixel_format()) {
				MessageBox(NULL, "Could not set pixel format", "Error", MB_OK);
				return -1;
			}

			ren_ctx = wglCreateContext(dev_ctx);

			if(!ren_ctx) {
				MessageBox(NULL, "Could not create GL render context.", "Error", MB_OK);
				return -1;
			}

			wglMakeCurrent(dev_ctx, ren_ctx);
			ShowWindow(wnd, SW_SHOW);
			return 0;

		case WM_LBUTTONDBLCLK:
			toggle_fullscreen(wnd);
			return 0;

		case WM_SYSCHAR:
			//ding ding ding
			return 0;

		case WM_SYSKEYDOWN:
			if(wparam == VK_RETURN && (lparam & (1<<29)))
				toggle_fullscreen(wnd);
			else
				keyboard[wparam] = 1;
			return 0;

		case WM_SYSKEYUP:
			keyboard[wparam] = 0;
			return 0;

		case WM_KEYDOWN:
			keyboard[wparam] = 1;

			if(wparam == VK_F1) { show_fps ^= 1; }
			if(wparam == VK_F2) { vsync ^= 1; wglSwapInterval(vsync); }

			if(wparam == VK_ESCAPE)
				PostMessage(wnd, WM_CLOSE, 0, 0);
			return 0;

		case WM_KEYUP:
			keyboard[wparam] = 0;
			return 0;

		case WM_SIZE:
			window_resize(LOWORD(lparam), HIWORD(lparam));
			return 0;

		case WM_CLOSE:
			wglMakeCurrent(NULL, NULL);

			if(ren_ctx) wglDeleteContext(ren_ctx);
			if(dev_ctx) ReleaseDC(wnd, dev_ctx);

			ren_ctx = NULL;
			dev_ctx = NULL;

			DestroyWindow(wnd);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(wnd, msg, wparam, lparam);
}

HWND create_window(HINSTANCE inst, int width, int height)
{
	RECT wnd_size;
	int wnd_width, wnd_height;

	char wnd_class[] = "LWINGS_CLASS";
	char wnd_title[] = "Legendary Wings";

	WNDCLASS wc;

	wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = wndproc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = inst;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = wnd_class;

	RegisterClass(&wc);

	wnd_size.left   = 0;
	wnd_size.top    = 0;
	wnd_size.right  = width;
	wnd_size.bottom = height;

	AdjustWindowRect(&wnd_size, WS_OVERLAPPEDWINDOW, FALSE);

	wnd_width  = wnd_size.right - wnd_size.left;
	wnd_height = wnd_size.bottom - wnd_size.top;

	return CreateWindow(wnd_class, wnd_title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, wnd_width, wnd_height,
		NULL, NULL, inst, NULL);
}

GLuint create_fb_texture(void)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

GLuint load_texture(char *filename)
{
	GLuint texture = 0;
	u32 width, height;
	u8 *data;

	lodepng_decode32_file(&data, &width, &height, filename);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	lodepng_free(data);

	return texture;
}

QWORD loop_level1, loop_boss;

void CALLBACK music_loop(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	BASS_ChannelSetPosition(channel, *(QWORD*)user, 0);
	BASS_ChannelPlay(channel, FALSE);
	//Beep(100,100);
}

void load_audio(void)
{
	int i;

	gameover_music = BASS_StreamCreateFile(FALSE, "data/snd/mus_game_over.mp3", 0, 0, 0);
	hiscore_music  = BASS_StreamCreateFile(FALSE, "data/snd/mus_score.mp3", 0, 0, 0);
	level1_music   = BASS_StreamCreateFile(FALSE, "data/snd/mus_stage1.mp3", 0, 0, BASS_STREAM_PRESCAN);
	warning_music  = BASS_StreamCreateFile(FALSE, "data/snd/mus_warning.mp3", 0, 0, BASS_SAMPLE_LOOP);
	boss_music     = BASS_StreamCreateFile(FALSE, "data/snd/mus_stage_boss.mp3", 0, 0, BASS_STREAM_PRESCAN);
	victory_music  = BASS_StreamCreateFile(FALSE, "data/snd/mus_victory.mp3", 0, 0, 0);

	loop_level1 = BASS_ChannelSeconds2Bytes(level1_music, 12.842);
	loop_boss   = BASS_ChannelSeconds2Bytes(boss_music, 17.103);

	BASS_ChannelSetSync(level1_music, BASS_SYNC_END, 0, music_loop, &loop_level1);
	BASS_ChannelSetSync(boss_music,   BASS_SYNC_END, 0, music_loop, &loop_boss);

	for(i = 0; i < 18; i++) {
		char buffer[128];
		wsprintf(buffer, "data/snd/snd_fx%02d.mp3", i);
		sndfx[i] = BASS_SampleLoad(FALSE, buffer, 0, 0, 1, 0);
		sndch[i] = BASS_SampleGetChannel(sndfx[i], FALSE);
	}
}

int get_fps_cmdline(char *cmdline, double *dt_fixed)
{
	char *s = cmdline;
	u32 num = 0;

	if(s) while(*s) {
		char c = *s++;
		if(c >= '0' && c <= '9') {
			u32 n = c - '0';
			num = num * 10 + n;
		}
	}

	if(num > 0 && num < 1000) {
		*dt_fixed = 1.0/(double)num;
		return 1;
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	int running = 1;
	LARGE_INTEGER time_query;
	DEVMODE display_settings;
	MSG msg;

	double dt_fixed = 1.0 / 60.0;
	double time_freq, time_old;

	HWND wnd = create_window(inst, 512, 480);
	if(!wnd) return -1;

	if(!get_fps_cmdline(cmdline, &dt_fixed) &&
		EnumDisplaySettings(NULL, 0, &display_settings))
			dt_fixed = 1.0 / display_settings.dmDisplayFrequency;

	wglSwapInterval = (WGLSWAPINTERVAL)wglGetProcAddress("wglSwapIntervalEXT");
	if(!wglSwapInterval) return -2;

	vsync = 1;
	wglSwapInterval(1);

	if (HIWORD(BASS_GetVersion())!=BASSVERSION ||
		!BASS_Init(-1, 44100, BASS_DEVICE_MONO, wnd, NULL))
		return -3;

	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC,  snd_vol);
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, snd_vol);
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, snd_vol);

 	load_audio();
	tex_atlas   = load_texture("data/atlas.png");
	tex_bullets = load_texture("data/bullets.png");
	tex_enemies = load_texture("data/enemies.png");
	tex_player  = load_texture("data/player.png");
	tex_fb      = create_fb_texture();

	glBindTexture(GL_TEXTURE_2D, tex_bullets);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_ALPHA_TEST);

	glAlphaFunc(GL_GREATER, 0.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	QueryPerformanceFrequency(&time_query);
	time_freq = (double)time_query.QuadPart;

	QueryPerformanceCounter(&time_query);
	time_old = (double)time_query.QuadPart;

	while(running) {
		double dt;

		QueryPerformanceCounter(&time_query);
		if(vsync) dt = dt_fixed;
		else dt = ((double)time_query.QuadPart - time_old) / time_freq;
		time_old = (double)time_query.QuadPart;

		glClear(GL_COLOR_BUFFER_BIT);

		glPushMatrix();
		glTranslatef(wnd_offx, wnd_offy, 0);
		glScalef(wnd_scale, wnd_scale, 1);
		lwings_game(dt);
		if(show_fps)
			draw_text(4, 4, "%04d FPS, %04d ms", (int)(1.0/dt), (int)(dt*1000.0));
		glPopMatrix();

		SwapBuffers(dev_ctx);

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				running = 0;
		}
	}

	BASS_Free();

	return 0;
}
