#NMAKE

CC_FLAGS = /O2 /MT /EHsc /Isrc /DUNICODE /nologo
LK_LIBS  = setupapi.lib user32.lib

all:
	@cl $(CC_FLAGS) src\TestClient.cpp src\ViGEmClient.cpp $(LK_LIBS) /FeViGEm
	@del *.obj

