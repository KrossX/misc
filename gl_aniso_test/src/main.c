/* Copyright (c) 2020 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "main.h"
#include <math.h>

//nVIDIA / AMD Performance Hints
//__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef BOOL (WINAPI *WGLSWAPINTERVAL)(int interval);
WGLSWAPINTERVAL wglSwapInterval;

HDC   dev_ctx;
HGLRC ren_ctx;

int wnd_wmin, wnd_hmin;
GLfloat wnd_width, wnd_height, wnd_scale, wnd_ox, wnd_oy;
GLuint tex_font, tex_zero, tex_mip1, tex_mipf;
int texture_current = 0;

int tex_min_filter = 0;
int tex_mag_filter = 0;
GLfloat tex_anisotropic = 1.0f;

int aniso_ext_found = 0;
int aniso_arb_found = 0;
int aniso_supported = 0;
float aniso_max = 1.0f;

char* tex_filter_str(int filter)
{
	switch(filter) {
		case 0: return "NEAREST";
		case 1: return "LINEAR ";
		case 2: return "N_MIP_N";
		case 3: return "L_MIP_N";
		case 4: return "N_MIP_L";
		case 5: return "L_MIP_L";
		default:return "UNK    ";
	}
}

GLuint tex_filter_val(int filter)
{
	switch(filter) {
		case 0: return GL_NEAREST;
		case 1: return GL_LINEAR;
		case 2: return GL_NEAREST_MIPMAP_NEAREST;
		case 3: return GL_LINEAR_MIPMAP_NEAREST;
		case 4: return GL_NEAREST_MIPMAP_LINEAR;
		case 5: return GL_LINEAR_MIPMAP_LINEAR;
		default: return 0;
	}
}

char* texture_str(int filter)
{
	switch(filter) {
		case 0: return "LEVEL0  ";
		case 1: return "LEVEL0+1";
		case 2: return "LEVELALL";
		default:return "UNK    ";
	}
}

GLuint texture_val(int filter)
{
	switch(filter) {
		case 0: return tex_zero;
		case 1: return tex_mip1;
		case 2: return tex_mipf;
		default: return 0;
	}
}

void toggle_fullscreen(HWND wnd)
{
	static WINDOWPLACEMENT prev = {sizeof(WINDOWPLACEMENT)};

	DWORD style = GetWindowLong(wnd, GWL_STYLE);

	if(style & WS_OVERLAPPEDWINDOW) {
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
	float sx = width/640.0f;
	float sy = height/480.0f;

	wnd_width  = (GLfloat)width;
	wnd_height = (GLfloat)height;
	wnd_scale  = sx < sy? sx : sy;

	glViewport(0, 0, width, height);
}

void proj_orthogonal(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, wnd_width, wnd_height, 0, -1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void proj_perspective(void)
{
	GLdouble fov    = 90.0 * 3.14159265358979323846 / 180.0;
	GLdouble znear  = 1;
	GLdouble aspect = (GLdouble)wnd_width / (GLdouble)wnd_height;
	GLdouble top    = znear * tan(fov/2.0);
	GLdouble bottom = -top;
	GLdouble right  = aspect * top;
	GLdouble left   = -right;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left, right, bottom, top, znear, 1000);
	glMatrixMode(GL_MODELVIEW);
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

int error_mb(char *msg, int ret)
{
	MessageBox(NULL, msg, "Error", MB_OK);
	return ret;
}

LRESULT CALLBACK wndproc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
	case WM_CREATE:
		dev_ctx = GetDC(wnd);

		if(!set_pixel_format())
			return error_mb("Could not set pixel format.", -1);

		ren_ctx = wglCreateContext(dev_ctx);

		if(!ren_ctx)
			return error_mb("Could not create GL render context.", -1);

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
		return 0;

	case WM_KEYDOWN:
		switch(wparam) {
		case VK_F1 :
			//tex_min_filter = texture_current == 2? (tex_min_filter+1)%6 : (tex_min_filter+1)%2;
			tex_min_filter = (tex_min_filter+1)%6;
			break;

		case VK_F2 :
			tex_mag_filter = (tex_mag_filter+1)%2;
			break;

		case VK_F3 :
			if(!aniso_supported) break;
			tex_anisotropic = tex_anisotropic + 1.0f;
			if(tex_anisotropic > aniso_max) tex_anisotropic = 1.0f;
			break;
		case VK_F4 :
			texture_current = (texture_current + 1)%3;
			//if(texture_current != 2 && tex_min_filter > 1) tex_min_filter = tex_min_filter%2;
			break;
		}

		if(wparam == VK_ESCAPE)
			PostMessage(wnd, WM_CLOSE, 0, 0);
		return 0;

	case WM_SIZING: {
		RECT *r = (RECT*)lparam;
		int w = r->right - r->left;
		int h = r->bottom - r->top;

		if(w < wnd_wmin) {
			w = wnd_wmin;
			if(wparam == WMSZ_RIGHT || wparam == WMSZ_TOPRIGHT ||
				wparam == WMSZ_BOTTOMRIGHT) r->right = r->left + w;
			else r->left = r->right - w;
		}

		if(h < wnd_hmin) {
			h = wnd_hmin;
			if(wparam == WMSZ_BOTTOM || wparam == WMSZ_BOTTOMLEFT ||
				wparam == WMSZ_BOTTOMRIGHT) r->bottom = r->top + h;
			else r->top = r->bottom - h;
		}

		} return 1;

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

	char wnd_class[] = "OGL_ANISO_TEST";
	char wnd_title[] = "OpenGL Anisotropic Test";

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

	wnd_wmin = wnd_width;
	wnd_hmin = wnd_height;

	return CreateWindow(wnd_class, wnd_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wnd_width, wnd_height, NULL, NULL, inst, NULL);
}

void draw_glyph(float x, float y, int charid)
{
	float tx1 = (charid&0xF)*8/128.0f;
	float ty1 = (charid>>4)*8/64.0f;
	float tx2 = tx1 + 8/128.0f;
	float ty2 = ty1 + 8/64.0f;

	float px1 = x;
	float py1 = y;
	float px2 = x + 8;
	float py2 = y + 8;

	GLfloat pos[4][2] = {{px1, py1}, {px1, py2}, {px2, py1}, {px2, py2}};
	GLfloat tex[4][2] = {{tx1, ty1}, {tx1, ty2}, {tx2, ty1}, {tx2, ty2}};

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2fv(tex[0]); glVertex2fv(pos[0]);
		glTexCoord2fv(tex[1]); glVertex2fv(pos[1]);
		glTexCoord2fv(tex[2]); glVertex2fv(pos[2]);
		glTexCoord2fv(tex[3]); glVertex2fv(pos[3]);
	glEnd();
}

void draw_string(float x, float y, char *fmt, ...)
{
	static char txtbuf[1024];
	char *str = txtbuf;
	float nx = x;
	float ny = y;

	va_list args;
	va_start(args, fmt);
	wvsprintf(txtbuf, fmt, args);
	va_end(args);

	while(*str) {
		char c = *str++;
		if(c > ' ' && c <= '~') {
			draw_glyph(nx, ny, c - ' ');
			nx += 8;
		} else if(c == ' ') {
			nx += 8;
		} else if(c == '\n') {
			nx  = x;
			ny += 8;
		}
	}
}

void check_error(void)
{
	char *str = "";
	GLenum err = glGetError();

	switch(err) {
	case GL_NO_ERROR:                      str = "GL_NO_ERROR"; break;
	case GL_INVALID_ENUM:                  str = "GL_INVALID_ENUM"; break;
	case GL_INVALID_VALUE:                 str = "GL_INVALID_VALUE"; break;
	case GL_INVALID_OPERATION:             str = "GL_INVALID_OPERATION"; break;
	//case GL_INVALID_FRAMEBUFFER_OPERATION: str = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
	case GL_OUT_OF_MEMORY:                 str = "GL_OUT_OF_MEMORY"; break;
	case GL_STACK_UNDERFLOW:               str = "GL_STACK_UNDERFLOW"; break;
	case GL_STACK_OVERFLOW:                str = "GL_STACK_OVERFLOW"; break;
	default: str = "UNKNOWN";
	}

	if(err != GL_NO_ERROR)
		MessageBoxA(NULL, str, "ERROR!", MB_OK);
}


void draw_texture(void)
{
	glColor4f(1,1,1,1);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(  0 * 8,   0 * 8); glVertex3f(-500,  -1,   -1);
		glTexCoord2f(  0 * 8, 800 * 8); glVertex3f(-500, 799, -901);
		glTexCoord2f(500 * 8,   0 * 8); glVertex3f( 500,  -1,   -1);
		glTexCoord2f(500 * 8, 800 * 8); glVertex3f( 500, 799, -901);
	glEnd();
}

#define TEXSIZE 8
u32 texbuf[TEXSIZE*TEXSIZE];

GLuint generate_texture(GLint max_level)
{
	u32 color[4][2] = {{0xFF000000, 0xFFFFFFFF},
					   {0xFF00007F, 0xFF7F7FFF},
					   {0xFF007F00, 0xFF7FFF7F},
					   {0xFF7F0000, 0xFFFF7F7F}};

	GLsizei size = TEXSIZE;
	GLint level = 0;
	int color_index = 0;

	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_filter_val(tex_mag_filter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_filter_val(tex_min_filter));
		if(aniso_supported) glTexParameterf(GL_TEXTURE_2D, 0x84FE, tex_anisotropic);

		while(size >= 1) {
			int sizeh = size/2;
			int x,y;
			for(y = 0; y < size; y++)
			for(x = 0; x < size; x++) {
				texbuf[y * size + x] = color[level&3][(x<sizeh)^(y<sizeh)];
			}

			glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, texbuf);
			size >>= 1;
			level++;

			if(max_level >= 0 && level > max_level)
				break;
		}

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}


void draw_debug_text_bg(float x, float y, float w, float h)
{
	float x1 = x;
	float y1 = y;
	float x2 = x1 + w;
	float y2 = y1 + h;

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glColor4f(0,0,0,0.85f);

	glBegin(GL_TRIANGLE_STRIP);
		glVertex2f(x1, y1);
		glVertex2f(x1, y2);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
	glEnd();

	glColor4f(0,0,0,0.45f);
	glBegin(GL_TRIANGLE_STRIP);
		glVertex2f(0, 0);
		glVertex2f(0, 24);
		glVertex2f(wnd_width, 0);
		glVertex2f(wnd_width, 24);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
}

void draw_debug_text(void)
{
	const char *vendor   = glGetString(GL_VENDOR);
	const char *renderer = glGetString(GL_RENDERER);
	const char *version  = glGetString(GL_VERSION);

	float bottom = (int)(wnd_height / wnd_scale) - 16.0f;
	float blacksz = bottom - 7 * 8.0f;
	int maxlen = 0;

	if(version) {
		int len = lstrlen(version) + 9;
		if(len > maxlen) maxlen = len;
	}

	if(renderer) {
		int len = lstrlen(renderer) + 10;
		if(len > maxlen) maxlen = len;
	}

	if(vendor) {
		int len = lstrlen(vendor) + 8;
		if(len > maxlen) maxlen = len;
	}

	if(maxlen < 41) maxlen = 41;
	draw_debug_text_bg(0, blacksz, maxlen * 8.0f + 16.0f, blacksz);
	glColor4f(1,1,0.3f,1);

	if(version) {
		draw_string(8,bottom,"VERSION: %s", version);
		bottom -= 8;
	}

	if(renderer) {
		draw_string(8,bottom,"RENDERER: %s", renderer);
		bottom -= 8;
	}

	if(vendor) {
		draw_string(8,bottom,"VENDOR: %s", vendor);
		bottom -= 8;
	}

	bottom -= 8;
	draw_string(8,bottom,"GL_EXT_texture_filter_anisotropic: %s", aniso_ext_found? "FOUND" : "NOPE"); bottom -= 8;
	draw_string(8,bottom,"GL_ARB_texture_filter_anisotropic: %s", aniso_arb_found? "FOUND" : "NOPE"); bottom -= 8;

	if(aniso_supported)
		draw_string(8,bottom,"Max anistropic level supported: %d", (int)aniso_max); bottom -= 8;
}

int find_string(const char *src, const char *str)
{
	const char *str0 = str;
	while(*src) {
		if(*src++ == *str++) {
			if(*str == 0) return 1;
		} else {
			str = str0;
		}
	}
	return 0;
}

void check_aniso_support(void)
{
	const char *extensions = glGetString(GL_EXTENSIONS);

	aniso_ext_found = find_string(extensions, "GL_EXT_texture_filter_anisotropic");
	aniso_arb_found = find_string(extensions, "GL_ARB_texture_filter_anisotropic");
	aniso_supported = aniso_ext_found | aniso_arb_found;

	if(aniso_supported) {
		glGetFloatv(0x84FF,&aniso_max);
		tex_anisotropic = aniso_max;
	}
}



int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	MSG msg;

	HWND wnd = create_window(inst, 640, 480);
	if(!wnd) return -1;

	//wglSwapInterval = (WGLSWAPINTERVAL)wglGetProcAddress("wglSwapIntervalEXT");
	//if(!wglSwapInterval) return -2;

	check_aniso_support();

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	tex_font = load_font_texture();
	tex_zero = generate_texture(0);
	tex_mip1 = generate_texture(1);
	tex_mipf = generate_texture(-1);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	while(1) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT) break;
		} else {
			glClear(GL_COLOR_BUFFER_BIT);

			proj_perspective();

			glBindTexture(GL_TEXTURE_2D, texture_val(texture_current));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex_filter_val(tex_min_filter));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex_filter_val(tex_mag_filter));
			if(aniso_supported)
				glTexParameterf(GL_TEXTURE_2D, 0x84FE, tex_anisotropic);

			draw_texture();

			glPushMatrix();
			glTranslatef(wnd_ox,wnd_oy,0);
			glScalef(wnd_scale,wnd_scale,1);
				proj_orthogonal();
				glBindTexture(GL_TEXTURE_2D, tex_font);
				draw_debug_text();
				glColor4f(1,1,0.7f,1);
				draw_string(8,8,"(F1)MIN:%s (F2)MAG:%s (F3)Aniso:%2d (F4)Tex:%s",
					tex_filter_str(tex_min_filter),
					tex_filter_str(tex_mag_filter),
					(int)tex_anisotropic,
					texture_str(texture_current));

				glColor4f(1,0.5f,0.5f,1);
				draw_string(40,8,"MIN             MAG             Aniso        Tex");
			glPopMatrix();
			//check_error();
			SwapBuffers(dev_ctx);
		}
	}

	return msg.wParam;
}



