/* Copyright (c) 2020 KrossX <krossx@live.com>
 * License: http://www.opensource.org/licenses/mit-license.html  MIT License
 */

#include "main.h"
#include <math.h>

#include <stddef.h>
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;
typedef char GLchar;

//nVIDIA / AMD Performance Hints
//__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef void (WINAPI *glGenBuffers_t) (GLsizei n, GLuint *buffers);
glGenBuffers_t glGenBuffers;

typedef void (WINAPI *glBindBuffer_t) (GLenum target, GLuint buffer);
glBindBuffer_t glBindBuffer;

typedef void (WINAPI *glBufferData_t) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
glBufferData_t glBufferData;

typedef void (WINAPI *glAttachShader_t) (GLuint program, GLuint shader);
glAttachShader_t glAttachShader;

typedef void (WINAPI *glCompileShader_t) (GLuint shader);
glCompileShader_t glCompileShader;

typedef GLuint (WINAPI *glCreateProgram_t) (void);
glCreateProgram_t glCreateProgram;

typedef GLuint (WINAPI *glCreateShader_t) (GLenum type);
glCreateShader_t glCreateShader;

typedef void (WINAPI *glDeleteProgram_t) (GLuint program);
glDeleteProgram_t glDeleteProgram;

typedef void (WINAPI *glDeleteShader_t) (GLuint shader);
glDeleteShader_t glDeleteShader;

typedef void (WINAPI *glEnableVertexAttribArray_t) (GLuint index);
glEnableVertexAttribArray_t glEnableVertexAttribArray;

typedef void (WINAPI *glGetProgramiv_t) (GLuint program, GLenum pname, GLint *params);
glGetProgramiv_t glGetProgramiv;

typedef void (WINAPI *glGetProgramInfoLog_t) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
glGetProgramInfoLog_t glGetProgramInfoLog;

typedef void (WINAPI *glGetShaderiv_t) (GLuint shader, GLenum pname, GLint *params);
glGetShaderiv_t glGetShaderiv;

typedef void (WINAPI *glGetShaderInfoLog_t) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
glGetShaderInfoLog_t glGetShaderInfoLog;

typedef void (WINAPI *glLinkProgram_t) (GLuint program);
glLinkProgram_t glLinkProgram;

typedef void (WINAPI *glShaderSource_t) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
glShaderSource_t glShaderSource;

typedef void (WINAPI *glUseProgram_t) (GLuint program);
glUseProgram_t glUseProgram;

typedef void (WINAPI *glUniform1f_t) (GLint location, GLfloat v0);
glUniform1f_t glUniform1f;

typedef void (WINAPI *glUniform2f_t) (GLint location, GLfloat v0, GLfloat v1);
glUniform2f_t glUniform2f;

typedef void (WINAPI *glUniform1i_t) (GLint location, GLint v0);
glUniform1i_t glUniform1i;

typedef GLubyte *(WINAPI *glGetStringi_t) (GLenum name, GLuint index);
glGetStringi_t glGetStringi;

typedef GLint (WINAPI *glGetUniformLocation_t) (GLuint program, const GLchar *name);
glGetUniformLocation_t glGetUniformLocation;

typedef void (WINAPI *glVertexAttribIPointer_t) (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
glVertexAttribIPointer_t glVertexAttribIPointer;

typedef void (WINAPI *glVertexAttribPointer_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
glVertexAttribPointer_t glVertexAttribPointer;

typedef void (WINAPI *glBindVertexArray_t) (GLuint array);
glBindVertexArray_t glBindVertexArray;

typedef void (WINAPI *glGenVertexArrays_t) (GLsizei n, GLuint *arrays);
glGenVertexArrays_t glGenVertexArrays;

typedef void (WINAPI *glMinSampleShading_t) (GLfloat value);
glMinSampleShading_t glMinSampleShading;

typedef BOOL (WINAPI *wglChoosePixelFormatARB_t) (HDC hdc, const int *iattr, const FLOAT *fattr, UINT maxf, int *pf, UINT *npf);
wglChoosePixelFormatARB_t wglChoosePixelFormatARB;

typedef HGLRC (WINAPI *wglCreateContextAttribsARB_t) (HDC hdc, HGLRC ctx, const int *attr_list);
wglCreateContextAttribsARB_t wglCreateContextAttribsARB;

typedef BOOL (WINAPI *wglSwapInterval_t) (int interval);
wglSwapInterval_t wglSwapInterval;

HDC   dev_ctx;
HGLRC ren_ctx;

int wnd_wmin, wnd_hmin;
GLfloat wnd_width, wnd_height, wnd_scale;
GLfloat wnd_aspect_x, wnd_aspect_y, wnd_sw, wnd_sh;
GLuint tex_font, tex_xor;
GLuint vbo_triangle, vao_triangle, vbo_text, vao_text, vbo_text_bg, vao_text_bg;
GLuint program_triangle, program_text, program_text_bg;
GLint uniform_aspect, uniform_angle, uniform_text_scale, uniform_text_bg_scale;

int arb_gpu5_found;
int arb_ubo_found;
int arb_ssbo_found;
int max_samples;

char txt_buffer[4096];

struct vertex {
	float x, y;
	u32 color;
};

struct vertex_text {
	float x, y;
	u32 color, id;
};

union shaderset {
	struct {
		u32 iblock : 1; // Using interface blocks
		u32 flat : 1; // Flat color qualifier
		u32 sample : 1; // Sample qualifier
	};
	
	u32 raw;
};

union shaderset sset, sset_old;

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
	float w, h;

	wnd_width  = w = (GLfloat)width;
	wnd_height = h = (GLfloat)height;
	wnd_scale  = sx < sy? sx : sy;

	glViewport(0, 0, width, height);

	if(wnd_width > wnd_height) {
		wnd_aspect_x = wnd_height / wnd_width;
		wnd_aspect_y = 1.0f;
	} else {
		wnd_aspect_y = wnd_width / wnd_height;
		wnd_aspect_x = 1.0f;
	}
	
	wnd_sw = (int)wnd_scale * 2.0f / wnd_width;
	wnd_sh = (int)wnd_scale * 2.0f / wnd_height;
}

int error_mb(char *msg, int ret)
{
	MessageBox(NULL, msg, "Error", MB_OK);
	return ret;
}

#define GETGLEXTENSION(var) \
	var = (var##_t)wglGetProcAddress(#var); \
	if(!var) return error_mb(#var " missing.", -1);

LRESULT CALLBACK wndproc_pre(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if(msg == WM_CREATE) {
		PIXELFORMATDESCRIPTOR pfd = {0};
		int pf;
		
		dev_ctx = GetDC(wnd);
		
		pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion        = 1;
		pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType      = PFD_TYPE_RGBA;
		pfd.cColorBits      = 32;
		pfd.iLayerType      = PFD_MAIN_PLANE;
		
		pf = ChoosePixelFormat(dev_ctx, &pfd);
		if(!pf) return error_mb("PRE: ChoosePixelFormat failed.", -1);
		
		if(!SetPixelFormat(dev_ctx, pf, &pfd))
			return error_mb("PRE: SetPixelFormat failed.", -1);
			
		ren_ctx = wglCreateContext(dev_ctx);
		if(!ren_ctx) return error_mb("PRE: wglCreateContext failed.", -1);
		
		wglMakeCurrent(dev_ctx, ren_ctx);
		
		GETGLEXTENSION(wglChoosePixelFormatARB);
		GETGLEXTENSION(wglCreateContextAttribsARB);
		
		wglMakeCurrent(NULL, NULL);

		wglDeleteContext(ren_ctx);
		ReleaseDC(wnd, dev_ctx);

		ren_ctx = NULL;
		dev_ctx = NULL;

		return 0; 
	}

	return DefWindowProc(wnd, msg, wparam, lparam);
}

#define WGL_DRAW_TO_WINDOW_ARB        0x2001
#define WGL_ACCELERATION_ARB          0x2003
#define WGL_SUPPORT_OPENGL_ARB        0x2010
#define WGL_DOUBLE_BUFFER_ARB         0x2011
#define WGL_PIXEL_TYPE_ARB            0x2013
#define WGL_COLOR_BITS_ARB            0x2014
#define WGL_DEPTH_BITS_ARB            0x2022
#define WGL_STENCIL_BITS_ARB          0x2023

#define WGL_FULL_ACCELERATION_ARB     0x2027
#define WGL_TYPE_RGBA_ARB             0x202B

#define WGL_SAMPLE_BUFFERS_ARB        0x2041
#define WGL_SAMPLES_ARB               0x2042

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092

#define GL_MULTISAMPLE                0x809D
#define GL_SAMPLE_SHADING             0x8C36

LRESULT CALLBACK wndproc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg) {
	case WM_CREATE: {
		PIXELFORMATDESCRIPTOR pfd = {0};
		int pf;
		UINT npf;
		
		const int pf_attr[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_SAMPLE_BUFFERS_ARB, 1,
			WGL_SAMPLES_ARB, 8,
			0
		};
		
		int  ctx_attr[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			0
		};
		
		dev_ctx = GetDC(wnd);
		if(!wglChoosePixelFormatARB(dev_ctx, pf_attr, NULL, 1, &pf, &npf) || !npf)
			return error_mb("wglChoosePixelFormatARB failed", -1);
			
		if(!DescribePixelFormat(dev_ctx, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd))
			return error_mb("DescribePixelFormat failed", -1);
		
		if(!SetPixelFormat(dev_ctx, pf, &pfd))
			return error_mb("SetPixelFormat failed", -1);

		ren_ctx = wglCreateContextAttribsARB(dev_ctx, NULL, ctx_attr);
		if(!ren_ctx) return error_mb("Could not create GL render context.", -1);

		wglMakeCurrent(dev_ctx, ren_ctx);
		
		GETGLEXTENSION(glGenBuffers);
		GETGLEXTENSION(glBindBuffer);
		GETGLEXTENSION(glBufferData);
		GETGLEXTENSION(glAttachShader);
		GETGLEXTENSION(glCompileShader);
		GETGLEXTENSION(glCreateProgram);
		GETGLEXTENSION(glCreateShader);
		GETGLEXTENSION(glDeleteProgram);
		GETGLEXTENSION(glDeleteShader);
		GETGLEXTENSION(glEnableVertexAttribArray);
		GETGLEXTENSION(glGetProgramiv);
		GETGLEXTENSION(glGetProgramInfoLog);
		GETGLEXTENSION(glGetShaderiv);
		GETGLEXTENSION(glGetShaderInfoLog);
		GETGLEXTENSION(glLinkProgram);
		GETGLEXTENSION(glShaderSource);
		GETGLEXTENSION(glUseProgram);
		GETGLEXTENSION(glUniform1f);
		GETGLEXTENSION(glUniform2f);
		GETGLEXTENSION(glUniform1i);
		GETGLEXTENSION(glGetStringi);
		GETGLEXTENSION(glGetUniformLocation);
		GETGLEXTENSION(glVertexAttribPointer);
		GETGLEXTENSION(glVertexAttribIPointer);
		GETGLEXTENSION(glBindVertexArray);
		GETGLEXTENSION(glGenVertexArrays);
		GETGLEXTENSION(glMinSampleShading);

		ShowWindow(wnd, SW_SHOW);
		} return 0;

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
		case VK_F1: sset.iblock ^= 1; break;
		case VK_F2: sset.flat ^= 1; break;
		case VK_F3: sset.sample ^= 1; break;
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
	HWND pre;
	RECT wnd_size;
	int wnd_width, wnd_height;

	char wnd_class[] = "OGL_SSAA_TEST";
	char wnd_title[] = "OpenGL SuperSampling Test";

	WNDCLASS wc;

	wc.style         = CS_DBLCLKS;
	wc.lpfnWndProc   = wndproc_pre;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = inst;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = wnd_class;

	RegisterClass(&wc);
	pre = CreateWindow(wnd_class, "Pre", 0, 0, 0, 1, 1, NULL, NULL, inst, NULL);
	if(!pre) return NULL;
	
	DestroyWindow(pre);
	UnregisterClass(wnd_class, inst);
	wc.lpfnWndProc = wndproc;
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

#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW  0x88E4
#define GL_STREAM_DRAW  0x88E0

#define PI 3.1415926535897932384626433832795
#define D2R(x) (PI * x / 180.0)

void prepare_triangle(void)
{
	struct vertex data[3] = {
		{(float)cos(D2R( 90)),(float)sin(D2R( 90)), 0x3030FF},
		{(float)cos(D2R(210)),(float)sin(D2R(210)), 0x30FF30},
		{(float)cos(D2R(330)),(float)sin(D2R(330)), 0xFF3030}
	};
	
	glGenVertexArrays(1, &vao_triangle);
	glBindVertexArray(vao_triangle);
	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(data[0]), (GLvoid*)0);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(data[0]), (GLvoid*)8);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_GEOMETRY_SHADER 0x8DD9

void dump_shader(char *str, char *err)
{
	char filename[64];
	int index = 0;
	HANDLE file;
	DWORD written;

	while(index < 9999) {
		wsprintf(filename, "dumpshader%04d.txt", index++);
		file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if(file != INVALID_HANDLE_VALUE) break;
		if(GetLastError() == ERROR_ALREADY_EXISTS) continue;
	}
	
	if(file == INVALID_HANDLE_VALUE) return;
	
	WriteFile(file, str, strlen(str), &written, NULL);
	WriteFile(file, err, strlen(err), &written, NULL);

	CloseHandle(file);
}

GLuint compile_shader(GLenum type, char *str)
{
	int success;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &str, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	
	if(!success) {
		static char buffer[4096];
		glGetShaderInfoLog(shader, 4096, NULL, buffer);
		MessageBoxA(NULL, buffer, "Shader compilation error!", MB_OK);
		dump_shader(str, buffer);
		glDeleteShader(shader);
		shader = 0;
	}
	
	return shader;
}

GLuint compile_program(int num, ...)
{
	int success;
	GLuint prog = glCreateProgram();
	
	va_list args;
	va_start(args, num);
	while(num--)
		glAttachShader(prog, va_arg(args, GLuint));
	va_end(args);
	
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	
	if(!success) {
		static char buffer[4096];
		glGetProgramInfoLog(prog, 4096, NULL, buffer);
		MessageBoxA(NULL, buffer, "Program linkage error!", MB_OK);
		glDeleteProgram(prog);
		prog = 0;
	}
	
	return prog;
}

void prepare_triangle_shader(void)
{
	GLuint vert, frag;
	char *txt_pos = txt_buffer;

	ZeroMemory(txt_buffer, 4096);
	txt_pos += wsprintf(txt_pos,
		"#version 420 core\n"
		"layout (location = 0) in vec2 pos;\n"
		"layout (location = 1) in uint col;\n");
		
	if(sset.iblock) {
		txt_pos += wsprintf(txt_pos,
			"out data {\n"
			"	%s vec4 color;\n"
			"	%s vec2 tcoord;\n"
			"};\n",
			sset.flat ? "flat" : sset.sample ? "sample" : "",
			sset.sample ? "sample" : "");
	} else {
		txt_pos += wsprintf(txt_pos,	
			"%s out vec4 color;\n"
			"%s out vec2 tcoord;\n",
			sset.flat ? "flat" : sset.sample ? "sample" : "",
			sset.sample ? "sample" : "");
	}

	txt_pos += wsprintf(txt_pos,
		"uniform vec2 aspect;\n"
		"uniform float angle;\n"
		"void main()\n"
		"{\n"
		"	vec2 npos;\n"
		"	npos.x = pos.x * cos(angle) - pos.y * sin(angle);\n"
		"	npos.y = pos.x * sin(angle) + pos.y * cos(angle);\n"
		"   gl_Position = vec4(npos * aspect, 0, 1);\n"
		"	color = vec4((col&0xFF)/255.0, ((col>>8)&0xFF)/255.0, ((col>>16)&0xFF)/255.0, 1);\n"
		"	tcoord = pos * 2.0;\n"
		"}\0");

	vert = compile_shader(GL_VERTEX_SHADER, txt_buffer);

	ZeroMemory(txt_buffer, 4096);
	txt_pos = txt_buffer;
	txt_pos += wsprintf(txt_pos,
		"#version 420 core\n"
		"out vec4 frag_color;\n");
		
	if(sset.iblock) {
		txt_pos += wsprintf(txt_pos,
			"in data {\n"
			"	%s vec4 color;\n"
			"	%s vec2 tcoord;\n"
			"};\n",
			sset.flat ? "flat" : sset.sample ? "sample" : "",
			sset.sample ? "sample" : "");
	} else {
		txt_pos += wsprintf(txt_pos,	
			"%s in vec4 color;\n"
			"%s in vec2 tcoord;\n",
			sset.flat ? "flat" : sset.sample ? "sample" : "",
			sset.sample ? "sample" : "");
	}
	
	txt_pos += wsprintf(txt_pos,
		"uniform sampler2D tex;\n"
		"void main()\n"
		"{\n"
		"	vec4 tcolor = vec4(texture(tex, tcoord).rrr, 1);\n"
		"   frag_color = color * (1 + tcolor) * 0.5;\n"
		"}\0");

	frag = compile_shader(GL_FRAGMENT_SHADER, txt_buffer);
	
	glDeleteProgram(program_triangle);
	program_triangle = compile_program(2, vert, frag);
	
	uniform_aspect = glGetUniformLocation(program_triangle, "aspect");
	uniform_angle = glGetUniformLocation(program_triangle, "angle");
	
	glDeleteShader(vert);
	glDeleteShader(frag);
}

void draw_triangle(void)
{
	static float angle;
	
	glBindTexture(GL_TEXTURE_2D, tex_xor);
	
	glUseProgram(program_triangle);
	glUniform1f(uniform_angle, (float)D2R(angle));
	glUniform2f(uniform_aspect, wnd_aspect_x, wnd_aspect_y);
	
	glBindVertexArray(vao_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	angle += 0.01f;
	if(angle > 360.0f) angle -= 360.0f;
}

char* gl_error_str(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "No error";
    case GL_INVALID_ENUM:      return "Invalid enum";
    case GL_INVALID_VALUE:     return "Invalid value";
    case GL_INVALID_OPERATION: return "Invalid operation";
    case GL_STACK_OVERFLOW:    return "Stack overflow";
    case GL_STACK_UNDERFLOW:   return "Stack underflow";
    case GL_OUT_OF_MEMORY:     return "Out of memory";
    default:                   return "Unknown error";
    }
}

void check_gl_error(void)
{
	while(1) {
		GLenum err = glGetError();
		if(err == GL_NO_ERROR) break;
		MessageBoxA(NULL, gl_error_str(err), "OpenGL Error!", MB_OK);
	}
}

GLuint generate_xor_texture(void)
{
	u8 buffer[16] = {0, -1, 0, -1, -1, 0, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0};
	GLuint texture = 0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return texture;
}

void prepare_text_shader(void)
{
	GLuint vert, frag, geom;
	
	ZeroMemory(txt_buffer, 4096);
	wsprintf(txt_buffer,
		"#version 420 core\n"
		"layout (location = 0) in vec2 in_pos;\n"
		"layout (location = 1) in uint in_col;\n"
		"layout (location = 2) in uint in_id;\n"
		"out vec4 v_color;\n"
		"out uint v_id;\n"
		"uniform vec2 scale;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(in_pos * scale + vec2(-1, 1), 0, 1);\n"
		"	v_color = vec4((in_col&0xFF)/255.0, ((in_col>>8)&0xFF)/255.0, ((in_col>>16)&0xFF)/255.0, 1);\n"
		"	v_id = in_id;\n"
		"}\0");
	vert = compile_shader(GL_VERTEX_SHADER, txt_buffer);
	
	ZeroMemory(txt_buffer, 4096);
	wsprintf(txt_buffer,
		"#version 420 core\n"
		"layout (points) in;\n"
		"layout (triangle_strip, max_vertices = 4) out;\n"
		"in vec4 v_color[];\n"
		"in uint v_id[];\n"
		"out vec4 fcolor;\n"
		"out vec2 tcoord;\n"
		"uniform vec2 scale;\n"
		"void main()\n"
		"{\n"
		"	vec4 texuv;\n"
		"	texuv.x = (v_id[0]&0xF)*8/128.0f;\n"
		"	texuv.y = (v_id[0]>>4)*8/64.0f;\n"
		"	texuv.z = texuv.x + 8/128.0f;\n"
		"	texuv.w = texuv.y + 8/64.0f;\n"
		"	fcolor = v_color[0];\n"
		"	tcoord = texuv.xy;\n"
		"	gl_Position = gl_in[0].gl_Position + vec4(0,0,0,0);\n"
		"	EmitVertex();\n"
		"	tcoord = texuv.zy;\n"
		"	gl_Position = gl_in[0].gl_Position + vec4(8*scale.x,0,0,0);\n"
		"	EmitVertex();\n"
		"	tcoord = texuv.xw;\n"
		"	gl_Position = gl_in[0].gl_Position + vec4(0,8*scale.y,0,0);\n"
		"	EmitVertex();\n"
		"	tcoord = texuv.zw;\n"
		"	gl_Position = gl_in[0].gl_Position + vec4(8*scale,0,0);\n"
		"	EmitVertex();\n"
		"	EndPrimitive();\n"
		"}\0");
	geom = compile_shader(GL_GEOMETRY_SHADER, txt_buffer);

	ZeroMemory(txt_buffer, 4096);
	wsprintf(txt_buffer,
		"#version 420 core\n"
		"out vec4 frag_color;\n"
		"in vec4 fcolor;\n"
		"in vec2 tcoord;\n"
		"uniform sampler2D tex;\n"
		"void main()\n"
		"{\n"
		"	float tcolor = texture(tex, tcoord).r;\n"
		"	if(tcolor < 0.5) discard;\n"
		"   frag_color = fcolor;\n"
		"}\0");
	frag = compile_shader(GL_FRAGMENT_SHADER, txt_buffer);

	glDeleteProgram(program_text);
	program_text = compile_program(3, vert, geom, frag);
	
	uniform_text_scale = glGetUniformLocation(program_text, "scale");
	
	glDeleteShader(geom);
	glDeleteShader(vert);
	glDeleteShader(frag);
}

void prepare_text_bg_shader(void)
{
	GLuint vert, frag;
	
	ZeroMemory(txt_buffer, 4096);
	wsprintf(txt_buffer,
		"#version 420 core\n"
		"layout (location = 0) in vec2 in_pos;\n"
		"uniform vec2 scale;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(in_pos * scale + vec2(-1, 1), 0, 1);\n"
		"}\0");
	vert = compile_shader(GL_VERTEX_SHADER, txt_buffer);
	
	ZeroMemory(txt_buffer, 4096);
	wsprintf(txt_buffer,
		"#version 420 core\n"
		"out vec4 frag_color;\n"
		"void main()\n"
		"{\n"
		"   frag_color = vec4(0,0,0,0.5);\n"
		"}\0");
	frag = compile_shader(GL_FRAGMENT_SHADER, txt_buffer);

	glDeleteProgram(program_text_bg);
	program_text_bg = compile_program(2, vert, frag);
	
	uniform_text_bg_scale = glGetUniformLocation(program_text_bg, "scale");
	
	glDeleteShader(vert);
	glDeleteShader(frag);
}

struct vertex_text vtxt_buff[1024];
u32 vtxt_count;

void prepare_text_buffers(void)
{
	glGenVertexArrays(1, &vao_text);
	glBindVertexArray(vao_text);
	glGenBuffers(1, &vbo_text);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtxt_buff), vtxt_buff, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vtxt_buff[0]), (GLvoid*)0);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(vtxt_buff[0]), (GLvoid*)8);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(vtxt_buff[0]), (GLvoid*)12);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void prepare_text_bg_buffers(void)
{
	glGenVertexArrays(1, &vao_text_bg);
	glBindVertexArray(vao_text_bg);
	glGenBuffers(1, &vbo_text_bg);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_text_bg);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, NULL, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

void text_add(float x, float y, u32 col, const char *fmt, ...)
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
			vtxt_buff[vtxt_count].x = nx;
			vtxt_buff[vtxt_count].y = ny;
			vtxt_buff[vtxt_count].color = col;
			vtxt_buff[vtxt_count].id = c - ' ';
			vtxt_count++;
			nx += 8;
		} else if(c == ' ') {
			nx += 8;
		} else if(c == '\n') {
			nx  = x;
			ny += 8;
		}
	}
}

void text_add_test(float x, float y, int test, const char *str_on, const char *str_off)
{
	if(test) text_add(x, y, 0x7FFF7F, str_on);
	else     text_add(x, y, 0x7F7FFF, str_off);
}

void draw_text_bg(float x1, float y1, float x2, float y2)
{
	float thingy[8] = {
		x1, y1,
		x1, y2,
		x2, y1,
		x2, y2 };

	glUseProgram(program_text_bg);
	glUniform2f(uniform_text_bg_scale, wnd_sw, -wnd_sh);
	
	glBindVertexArray(vao_text_bg);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_text_bg);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, thingy, GL_STREAM_DRAW);
	
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);
}

void setup_text(void)
{
	const char *vendor   = glGetString(GL_VENDOR);
	const char *renderer = glGetString(GL_RENDERER);
	const char *version  = glGetString(GL_VERSION);

	float bottom = wnd_height / (int)wnd_scale - 16.0f;
	int maxlen = 0;
	
	draw_text_bg(0, 0, wnd_width, 24);
	
	vtxt_count = 0;
	text_add(8, 8, 0xAFFFFF, "(  )Interface Block:     (  )Flat Qualifier:     (  )Sample Qualifier:    ");
	text_add(8, 8, 0xFFFFFF, " F1                       F2                      F3                      ");

	text_add_test(176, 8, sset.iblock, "ON", "OFF");
	text_add_test(368, 8, sset.flat,   "ON", "OFF");
	text_add_test(576, 8, sset.sample, "ON", "OFF");
	
	draw_text_bg(0, bottom - 8 * 8.0f, wnd_width, wnd_height);
	
	if(version) {
		text_add(8, bottom, 0xFFFFFF, "VERSION");
		text_add(80, bottom, 0xAFFFFF, version);
		bottom -= 8;
	}

	if(renderer) {
		text_add(8, bottom, 0xFFFFFF, "RENDERER");
		text_add(80, bottom, 0xAFFFFF, renderer);
		bottom -= 8;
	}

	if(vendor) {
		text_add(8, bottom, 0xFFFFFF, "VENDOR");
		text_add(80, bottom, 0xAFFFFF, vendor);
		bottom -= 8;
	}
	
	bottom -= 8;
	
	text_add(8, bottom, 0xAFFFFF, "ARB_shader_storage_buffer_object:");
	text_add_test(280, bottom, arb_ssbo_found, "FOUND", "NOPE");
	bottom -= 8;
	
	text_add(8, bottom, 0xAFFFFF, "ARB_uniform_buffer_object:");
	text_add_test(224, bottom, arb_ubo_found, "FOUND", "NOPE");
	bottom -= 8;
	
	text_add(8, bottom, 0xAFFFFF, "ARB_gpu_shader5:");
	text_add_test(144, bottom, arb_gpu5_found, "FOUND", "NOPE");
	bottom -= 8;
	
	text_add(8, bottom, 0xAFFFFF, "Max Samples Supported:");
	text_add(192, bottom, 0xFFFFFF, "%d", max_samples);
	bottom -= 8;
}

void draw_text(void)
{
	setup_text();
	
	glBindTexture(GL_TEXTURE_2D, tex_font);
	glUseProgram(program_text);
	
	glUniform2f(uniform_text_scale, wnd_sw, -wnd_sh);
	
	glBindVertexArray(vao_text);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_text);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtxt_buff), vtxt_buff, GL_STREAM_DRAW);
	glDrawArrays(GL_POINTS, 0, vtxt_count);
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

void check_ext_support(void)
{
	u32 index = 0;
	
	while(1) {
		const char *extensions = glGetStringi(GL_EXTENSIONS, index++);
		GLenum err = glGetError();
		if(!extensions || err == GL_INVALID_VALUE) break;

		arb_gpu5_found |= find_string(extensions, "GL_ARB_gpu_shader5");
		arb_ubo_found  |= find_string(extensions, "GL_ARB_uniform_buffer_object");
		arb_ssbo_found |= find_string(extensions, "GL_ARB_shader_storage_buffer_object");
	}
	
	glGetIntegerv(0x8D57, &max_samples);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	MSG msg;

	HWND wnd = create_window(inst, 640, 480);
	if(!wnd) return -1;
	
	check_gl_error();
	check_ext_support();

	wglSwapInterval = (wglSwapInterval_t)wglGetProcAddress("wglSwapIntervalEXT");
	if(!wglSwapInterval) return -2;
	wglSwapInterval(1);
	
	tex_font = load_font_texture(); 
	tex_xor = generate_xor_texture();

	prepare_text_bg_shader();
	prepare_text_bg_buffers();
	prepare_text_shader();
	prepare_text_buffers();

	prepare_triangle();
	sset.sample = 1;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	check_gl_error();
	
	while(1) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			//TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT) break;
		} else {
			glClear(GL_COLOR_BUFFER_BIT);
			
			if(sset.raw != sset_old.raw) {
				prepare_triangle_shader();
				sset_old.raw = sset.raw;
			}
			
			draw_triangle();
			draw_text();
			
			SwapBuffers(dev_ctx);
		}
	}

	return msg.wParam;
}



