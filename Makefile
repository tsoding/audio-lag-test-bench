PKGS=openal freealut sdl2
CFLAGS=$(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -ggdb -Wno-deprecated-declarations

all: probe-no-vsync-openal probe-vsync-openal probe-no-vsync-sdl-queued probe-vsync-sdl-queued probe-no-vsync-sdl-callback probe-vsync-sdl-callback

probe-vsync-openal: main.c
	$(CC) $(CFLAGS) -DENABLE_VSYNC -o $@ main.c $(LIBS)

probe-no-vsync-openal: main.c
	$(CC) $(CFLAGS) -o $@ main.c $(LIBS)

probe-vsync-sdl-queued: main.c
	$(CC) $(CFLAGS) -DSOUND=1 -DENABLE_VSYNC -o $@ main.c $(LIBS)

probe-no-vsync-sdl-queued: main.c
	$(CC) $(CFLAGS) -DSOUND=1 -o $@ main.c $(LIBS)

probe-vsync-sdl-callback: main.c
	$(CC) $(CFLAGS) -DSOUND=2 -DENABLE_VSYNC -o $@ main.c $(LIBS)

probe-no-vsync-sdl-callback: main.c
	$(CC) $(CFLAGS) -DSOUND=2 -o $@ main.c $(LIBS)
