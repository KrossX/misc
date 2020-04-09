#NMAKE

CFLAGS = /c /O2 /GR- /MD /nologo /W3
LINKFLAGS = /SUBSYSTEM:WINDOWS /NOLOGO
LINKLIBS = user32.lib gdi32.lib opengl32.lib

all: lwings.exe

clean:
	@del obj\*.obj

lwings.exe: obj/lwings_base.obj obj/lwings_game.obj obj/lwings_draw.obj
	@link $** obj/lodepng.lib obj/bass.lib $(LINKLIBS) $(LINKFLAGS) /out:$@
	@del *.exp
	@del *.lib

	
{src}.c{obj}.obj:
	@cl $(CFLAGS) $< -Fo$@