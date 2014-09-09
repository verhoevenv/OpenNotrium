# This is a custom Makefile for building Notrium.
# Pass the resulting executable to a proper Notrium directory
# (containing the folders 'data', 'save', 'textures',...)

# The following libraries are required:
# SDL
# SDL_mixer
# SDL_image
# physfs

CC = g++
CFLAGS_SDL = $(shell sdl-config --cflags)
OBJS = WinMain.o editor.o engine.o entities.o \
	func.o memleaks.o mod_loader.o puzzle.o \
	resource_handler.o sinecosine.o soundmanager.o \
	text_output.o
CFLAGS = $(CFLAGS_SDL)
LFLAGS = $(shell sdl-config --libs) -lGL -lSDL_mixer -lSDL_image -lphysfs

CFLAGS_PARANOID = -g -DSDL_ASSERT_LEVEL=3
CFLAGS_DEBUG = -g -DSDL_ASSERT_LEVEL=2
CFLAGS_RELEASE = -s -O2 -DSDL_ASSERT_LEVEL=1

all: release

release: CFLAGS += $(CFLAGS_RELEASE)
release: notrium

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: notrium

paranoid: CFLAGS += $(CFLAGS_PARANOID)
paranoid: notrium

notrium: $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

.cpp.o: 
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
		rm -f *.o notrium
