#NMAKE

CFLAGS = /c /O2 /GR- /MD /nologo /W3
LINKFLAGS = /SUBSYSTEM:WINDOWS /NOLOGO
LINKLIBS = user32.lib gdi32.lib opengl32.lib

all: gl_aniso_test.exe

clean:
	@del obj\*.obj

gl_aniso_test.exe: obj/main.obj obj/font.obj
	@link $** $(LINKLIBS) $(LINKFLAGS) /out:$@

{src}.c{obj}.obj:
	@cl $(CFLAGS) $< -Fo$@
