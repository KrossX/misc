#pragma warning(push, 1)
#define WINVER         0x0400
#define _WIN32_WINNT   0x0400
#define _WIN32_WINDOWS 0x0400
#define _WIN32_IE      0x0400

#include <windows.h>
#include <gl/gl.h>
#pragma warning(pop)

#include <math.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")

typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

//nVIDIA / AMD Performance Hints
//__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef BOOL (WINAPI *WGLSWAPINTERVAL)(int interval);
WGLSWAPINTERVAL wglSwapInterval;

HDC   dev_ctx;
HGLRC ren_ctx;

GLfloat wnd_width, wnd_height;

GLfloat rgb1[4] = {1,1,1,1};
GLfloat rgb0[4] = {0,0,0,1};

u8 key[256];
int walking;
float pos_x, pos_y, pos_angle;
float anim;

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
	GLdouble fov    = 90.0 * 3.14159265358979323846 / 180.0;
	GLdouble znear  = 1;
	GLdouble aspect = (GLdouble)width / (GLdouble)height;
	GLdouble top    = znear * 1; //* tan(fov/2.0); // include <math.h>
	GLdouble bottom = -top;
	GLdouble right  = aspect * top;
	GLdouble left   = -right;

	float r = right;
	float t = top;

	GLfloat mat4x4[] = {
		1/r, 0, 0, 0,
		0, 1/t, 0, 0,
		0, 0,-0.1, 0,
		0, 0, 0, 1};

	wnd_width  = (GLfloat)width;
	wnd_height = (GLfloat)height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(mat4x4);
	//glFrustum(left, right, bottom, top, znear, 1000);
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
		key[wparam&0xFF] = 1;
		if(wparam == VK_ESCAPE)
			PostMessage(wnd, WM_CLOSE, 0, 0);
		return 0;

	case WM_KEYUP:
		key[wparam&0xFF] = 0;
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

	char wnd_class[] = "DUCKY_CLASS";
	char wnd_title[] = "Ducky";

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

	return CreateWindow(wnd_class, wnd_title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		wnd_size.right - wnd_size.left, wnd_size.bottom - wnd_size.top, NULL, NULL, inst, NULL);
}

struct vec3 { float x,y,z; };

void draw_cube(struct vec3 s, struct vec3 p)
{
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(-0.1,-1,-0.1); glVertex3f(-s.x+p.x,-s.y+p.y,-s.z+p.z);
		glNormal3f(-0.1,-1, 0.1); glVertex3f(-s.x+p.x,-s.y+p.y, s.z+p.z);
		glNormal3f( 0.1,-1,-0.1); glVertex3f( s.x+p.x,-s.y+p.y,-s.z+p.z);
		glNormal3f( 0.1,-1, 0.1); glVertex3f( s.x+p.x,-s.y+p.y, s.z+p.z);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f( 0.1,-0.1, 1); glVertex3f( s.x+p.x,-s.y+p.y, s.z+p.z);
		glNormal3f( 0.1, 0.1, 1); glVertex3f( s.x+p.x, s.y+p.y, s.z+p.z);
		glNormal3f(-0.1,-0.1, 1); glVertex3f(-s.x+p.x,-s.y+p.y, s.z+p.z);
		glNormal3f(-0.1, 0.1, 1); glVertex3f(-s.x+p.x, s.y+p.y, s.z+p.z);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(-1,-0.1, 0.1); glVertex3f(-s.x+p.x,-s.y+p.y, s.z+p.z);
		glNormal3f(-1, 0.1, 0.1); glVertex3f(-s.x+p.x, s.y+p.y, s.z+p.z);
		glNormal3f(-1,-0.1,-0.1); glVertex3f(-s.x+p.x,-s.y+p.y,-s.z+p.z);
		glNormal3f(-1, 0.1,-0.1); glVertex3f(-s.x+p.x, s.y+p.y,-s.z+p.z);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(-0.1,-0.1,-1); glVertex3f(-s.x+p.x,-s.y+p.y,-s.z+p.z);
		glNormal3f(-0.1, 0.1,-1); glVertex3f(-s.x+p.x, s.y+p.y,-s.z+p.z);
		glNormal3f( 0.1,-0.1,-1); glVertex3f( s.x+p.x,-s.y+p.y,-s.z+p.z);
		glNormal3f( 0.1, 0.1,-1); glVertex3f( s.x+p.x, s.y+p.y,-s.z+p.z);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f( 1,-0.1,-0.1); glVertex3f( s.x+p.x,-s.y+p.y,-s.z+p.z);
		glNormal3f( 1, 0.1,-0.1); glVertex3f( s.x+p.x, s.y+p.y,-s.z+p.z);
		glNormal3f( 1,-0.1, 0.1); glVertex3f( s.x+p.x,-s.y+p.y, s.z+p.z);
		glNormal3f( 1, 0.1, 0.1); glVertex3f( s.x+p.x, s.y+p.y, s.z+p.z);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f( 0.1, 1, 0.1); glVertex3f( s.x+p.x, s.y+p.y, s.z+p.z);
		glNormal3f( 0.1, 1,-0.1); glVertex3f( s.x+p.x, s.y+p.y,-s.z+p.z);
		glNormal3f(-0.1, 1, 0.1); glVertex3f(-s.x+p.x, s.y+p.y, s.z+p.z);
		glNormal3f(-0.1, 1,-0.1); glVertex3f(-s.x+p.x, s.y+p.y,-s.z+p.z);
	glEnd();
}

void draw_eyes(struct vec3 s, struct vec3 p)
{
	float k = 0.35f;

	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f( 1,-0.1,-0.1); glVertex3f( s.x+p.x,-s.y+p.y,-s.z+p.z-k);
		glNormal3f( 1, 0.1,-0.1); glVertex3f( s.x+p.x, s.y+p.y,-s.z+p.z-k);
		glNormal3f( 1,-0.1, 0.1); glVertex3f( s.x+p.x,-s.y+p.y, s.z+p.z-k);
		glNormal3f( 1, 0.1, 0.1); glVertex3f( s.x+p.x, s.y+p.y, s.z+p.z-k);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glNormal3f( 1,-0.1,-0.1); glVertex3f( s.x+p.x,-s.y+p.y,-s.z+p.z+k);
		glNormal3f( 1, 0.1,-0.1); glVertex3f( s.x+p.x, s.y+p.y,-s.z+p.z+k);
		glNormal3f( 1,-0.1, 0.1); glVertex3f( s.x+p.x,-s.y+p.y, s.z+p.z+k);
		glNormal3f( 1, 0.1, 0.1); glVertex3f( s.x+p.x, s.y+p.y, s.z+p.z+k);
	glEnd();
}

void draw_duck(void)
{
	struct cube { struct vec3 size, pos; };
	struct cube body = {1,1,1,0,0,0};
	struct cube tail = {0.25f,0.25f,0.25f,-1.25f,1.25f,0};
	struct cube head = {0.75f,0.75f,0.75f,1.0f,1.75f,0};
	struct cube eyes = {0.15f,0.15f,0.15f,1.61f,1.90f,0};
	struct cube beak = {0.45f,0.10f,0.45f,2.20f,1.45f,0};
	struct cube legl = {0.20f,0.40f,0.20f,0.05f,-1.40f, 0.6f};
	struct cube ftlt = {0.40f,0.20f,0.20f,0.25f,-2.00f, 0.6f};
	struct cube legr = {0.20f,0.40f,0.20f,0.05f,-1.40f,-0.6f};
	struct cube ftrt = {0.40f,0.20f,0.20f,0.25f,-2.00f,-0.6f};

	float matbody[4] = {1,194/255.0f,0,1};
	float mateyes[4] = {0.2f,0.2f,0.2f,1};
	float matbeak[4] = {1,103/255.0f,0,1};

	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matbody);
	glColor4fv(matbody);

	glPushMatrix();
		if(walking) {
			glTranslatef(1, 0, 0);
			glRotatef(sin(anim*18)*15,0,1,0);
			glRotatef(-5,0,0,1);
			glTranslatef(-1, 0, 0);
		}
		draw_cube(body.size, body.pos);
		draw_cube(tail.size, tail.pos);
	glPopMatrix();

	glPushMatrix();
		if(walking) {
			glTranslatef(0.35, 0, 0);
		}
		draw_cube(head.size, head.pos);

		//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mateyes);
		glColor4fv(mateyes);
		draw_eyes(eyes.size, eyes.pos);

		//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matbeak);
		glColor4fv(matbeak);
		draw_cube(beak.size, beak.pos);
	glPopMatrix();
	
	glPushMatrix();
		if(walking) {
			glTranslatef(sin(anim*18)*0.2, 0, 0);
		}
		draw_cube(legl.size, legl.pos);
		draw_cube(ftlt.size, ftlt.pos);
	glPopMatrix();

	glPushMatrix();
		if(walking) {
			glTranslatef(-sin(anim*18)*0.2, 0, 0);
		}
		draw_cube(legl.size, legr.pos);
		draw_cube(ftlt.size, ftrt.pos);
	glPopMatrix();

	//glRotatef( 45,1,0,0);
	//glRotatef(-45,0,1,0);
	//glTranslatef(cam_pos,cam_pos,cam_pos);
	//glTranslatef(pos_x, 0, -pos_y);
	//glScalef(0.1f,0.1f,0.1f);
	//glRotatef(pos_angle, 0, 1, 0);
	//draw_duck();
}

void setup_lighting(void)
{
	float lightpos[4] = {0,-20,100,1};
	float lightamb[4] = {0.2,0,0,1};
	float lightdif[4] = {1,1,1,1};
	float specular[4] = {1,0.7,0.7,1};

	//glShadeModel(GL_FLAT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, rgb0);
	glMateriali(GL_FRONT, GL_SHININESS, 3);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdif);
	glLightfv(GL_LIGHT0, GL_SPECULAR, rgb1);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, rgb0);

	//glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
	//glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1);
	//glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 1);
}

float length(float x, float y)
{
	return sqrt(x*x+y*y);
}

void check_input(double dt)
{
	float delta_x = 0;
	float delta_y = 0;
	float len;

	delta_x += key[VK_RIGHT];
	delta_x -= key[VK_LEFT];
	delta_y += key[VK_UP];
	delta_y -= key[VK_DOWN];

	walking = 0;
	if(delta_x == 0 && delta_y == 0)
		return;
	walking = 1;

	len = length(delta_x, delta_y);
	delta_x /= len;
	delta_y /= len;

	anim += dt;

	pos_angle = atan2(delta_y, delta_x) * 180 / 3.1415926535897932384626433832795;

	pos_x += delta_x * dt;
	pos_y += delta_y * dt;

	len = length(pos_x, pos_y);
	if(len > 1) { pos_x /= len; pos_y /= len; };
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	float cam_pos = -sqrt(1/3.0);
	MSG msg;

	HWND wnd = create_window(inst, 640, 480);
	if(!wnd) return -1;

	wglSwapInterval = (WGLSWAPINTERVAL)wglGetProcAddress("wglSwapIntervalEXT");
	if(!wglSwapInterval) return -2;
	wglSwapInterval(1);

	setup_lighting();

	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	while(1) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT) break;
		} else {
			check_input(1/60.0);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPushMatrix();
				glRotatef( 45,1,0,0);
				glRotatef(-45,0,1,0);
				glTranslatef(cam_pos,cam_pos,cam_pos);
				glTranslatef(pos_x, 0, -pos_y);
				glScalef(0.1f,0.1f,0.1f);
				glRotatef(pos_angle, 0, 1, 0);
				draw_duck();
			glPopMatrix();

			SwapBuffers(dev_ctx);
		}
	}

	return msg.wParam;
}
