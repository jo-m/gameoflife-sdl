CC = gcc

INCLUDEDIR = $(shell dirname $$(brew ls sdl | grep /include/ | head -n 1))
LIBDIR = $(shell dirname $$(brew ls sdl | grep /lib/ | head -n 1))
INCLUDEDIR_IMG = $(shell dirname $$(brew ls sdl_image | grep /include/ | head -n 1))
LIBDIR_IMG=$(shell dirname $$(brew ls sdl_image | grep /lib/ | head -n 1))
LIBS = -lSDL -lSDL_image -lSDLmain -L$(LIBDIR) -L$(LIBDIR_IMG) -I$(INCLUDEDIR) -I$(INCLUDEDIR_IMG)
FRAMEWORKS = -framework Cocoa -framework ApplicationServices

gol: gol.c
	$(CC) gol.c -o gol $(LIBS) $(FRAMEWORKS)
