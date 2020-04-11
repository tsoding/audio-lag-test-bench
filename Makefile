PKGS=openal freealut sdl2
CFLAGS=$(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -ggdb

probe: main.c
	$(CC) $(CFLAGS) -o probe main.c $(LIBS)
