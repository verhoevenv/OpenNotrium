bin_PROGRAMS = notrium
AM_CXXFLAGS = -Wall $(shell sdl-config --cflags)

notrium_SOURCES = WinMain.cpp \
  blendstate.h editor.cpp editor.h engine.cpp engine.h \
  entities.cpp entities.h func.cpp func.h \
  keys.h main.h memleaks.cpp memleaks.h \
  mod_loader.cpp mod_loader.h musicmanager.h puzzle.cpp \
  puzzle.h resource.h resource_handler.cpp resource_handler.h \
  sinecosine.cpp sinecosine.h soundmanager.cpp soundmanager.h \
  text_output.cpp text_output.h


notrium_LDFLAGS = $(shell sdl-config --libs) -lSDL_mixer -lSDL_image -lGL -lphysfs

