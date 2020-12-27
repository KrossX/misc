#NMAKE

CFLAGS = /O2 /GR- /MD /nologo /W3
LINKFLAGS = /SUBSYSTEM:WINDOWS /NOLOGO
LINKLIBS = advapi32.lib user32.lib

all:
	@cl eemb.c $(CFLAGS) /link $(LINKFLAGS) $(LINKLIBS) 
	@del *.obj
