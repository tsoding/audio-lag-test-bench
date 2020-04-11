PKGS=openal freealut sdl2
CFLAGS=$(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -ggdb -Wno-deprecated-declarations

all: probe-no-vsync probe-vsync

probe-vsync: main.c
	$(CC) $(CFLAGS) -DENABLE_VSYNC -o probe-vsync main.c $(LIBS)

probe-no-vsync: main.c
	$(CC) $(CFLAGS) -o probe-no-vsync main.c $(LIBS)
